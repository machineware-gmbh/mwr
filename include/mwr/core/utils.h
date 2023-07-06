/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_H
#define MWR_UTILS_H

#include <string>
#include <vector>

#include "mwr/core/types.h"

namespace mwr {

using std::string;
using std::vector;

bool file_exists(const string& file);
bool directory_exists(const string& file);

string dirname(const string& path);
string filename(const string& path);
string filename_noext(const string& path);

string curr_dir();
string temp_dir();

string progname();
string username();

vector<string> backtrace(size_t frames = 63, size_t skip = 1);
void report_segfaults();
extern size_t max_backtrace_length;

size_t fd_peek(int fd, time_t timeout_ms = 0);
size_t fd_read(int fd, void* buffer, size_t buflen);
size_t fd_write(int fd, const void* buffer, size_t buflen);

double timestamp();
u64 timestamp_ms();
u64 timestamp_us();
u64 timestamp_ns();

inline void cpu_yield() {
#if defined(__x86_64__)
    asm volatile("pause" ::: "memory");
#elif defined(__aarch64__)
    asm volatile("yield" ::: "memory");
#endif
}

} // namespace mwr

#endif
