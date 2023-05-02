/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_TERMINAL_H
#define MWR_TERMINAL_H

namespace mwr {

struct termcolors {
    static const char* const CLEAR;
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
