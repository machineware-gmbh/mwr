/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_TERMINAL_H
#define MWR_UTILS_TERMINAL_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"
#include "mwr/core/compiler.h"
#include "mwr/core/utils.h"
#include "mwr/core/bitfields.h"

namespace mwr {

int new_tty();
bool is_tty(int fd);

bool tty_is_vt100(int fd);
void tty_setup_vt100(int fd);

void tty_push(int fd, bool restore);
void tty_pop(int fd);

class tty_guard
{
private:
    int m_fd;

public:
    tty_guard(int fd): m_fd(fd) { tty_push(m_fd, true); }
    virtual ~tty_guard() { tty_pop(m_fd); }
};

struct termcolors {
    static constexpr const char* CLEAR = "\x1b[0m";
    static constexpr const char* BOLD = "\x1b[1m";
    static constexpr const char* BLACK = "\x1b[30m";
    static constexpr const char* RED = "\x1b[31m";
    static constexpr const char* GREEN = "\x1b[32m";
    static constexpr const char* YELLOW = "\x1b[33m";
    static constexpr const char* BLUE = "\x1b[34m";
    static constexpr const char* MAGENTA = "\x1b[35m";
    static constexpr const char* CYAN = "\x1b[36m";
    static constexpr const char* WHITE = "\x1b[37m";
    static constexpr const char* BRIGHT_BLACK = "\x1b[90m";
    static constexpr const char* BRIGHT_RED = "\x1b[91m";
    static constexpr const char* BRIGHT_GREEN = "\x1b[92m";
    static constexpr const char* BRIGHT_YELLOW = "\x1b[93m";
    static constexpr const char* BRIGHT_BLUE = "\x1b[94m";
    static constexpr const char* BRIGHT_MAGENTA = "\x1b[95m";
    static constexpr const char* BRIGHT_CYAN = "\x1b[96m";
    static constexpr const char* BRIGHT_WHITE = "\x1b[97m";
};

} // namespace mwr

#endif
