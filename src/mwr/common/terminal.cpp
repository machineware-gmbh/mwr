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

#include "mwr/common/terminal.h"

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
