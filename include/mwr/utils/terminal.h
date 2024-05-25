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
    static const char* const CLEAR;
    static const char* const BOLD;
    static const char* const BLACK;
    static const char* const RED;
    static const char* const GREEN;
    static const char* const YELLOW;
    static const char* const BLUE;
    static const char* const MAGENTA;
    static const char* const CYAN;
    static const char* const WHITE;
    static const char* const BRIGHT_BLACK;
    static const char* const BRIGHT_RED;
    static const char* const BRIGHT_GREEN;
    static const char* const BRIGHT_YELLOW;
    static const char* const BRIGHT_BLUE;
    static const char* const BRIGHT_MAGENTA;
    static const char* const BRIGHT_CYAN;
    static const char* const BRIGHT_WHITE;
};

} // namespace mwr

#endif
