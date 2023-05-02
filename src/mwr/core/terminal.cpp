/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/core/terminal.h"

namespace mwr {

const char* const termcolors::CLEAR = "\x1b[0m";
const char* const termcolors::BLACK = "\x1b[30m";
const char* const termcolors::RED = "\x1b[31m";
const char* const termcolors::GREEN = "\x1b[32m";
const char* const termcolors::YELLOW = "\x1b[33m";
const char* const termcolors::BLUE = "\x1b[34m";
const char* const termcolors::MAGENTA = "\x1b[35m";
const char* const termcolors::CYAN = "\x1b[36m";
const char* const termcolors::WHITE = "\x1b[37m";
const char* const termcolors::BRIGHT_BLACK = "\x1b[90m";
const char* const termcolors::BRIGHT_RED = "\x1b[91m";
const char* const termcolors::BRIGHT_GREEN = "\x1b[92m";
const char* const termcolors::BRIGHT_YELLOW = "\x1b[93m";
const char* const termcolors::BRIGHT_BLUE = "\x1b[94m";
const char* const termcolors::BRIGHT_MAGENTA = "\x1b[95m";
const char* const termcolors::BRIGHT_CYAN = "\x1b[96m";
const char* const termcolors::BRIGHT_WHITE = "\x1b[97m";

} // namespace mwr
