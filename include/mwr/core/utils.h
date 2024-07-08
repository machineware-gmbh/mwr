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
#include <optional>

#include "mwr/core/compiler.h"
#include "mwr/core/types.h"

namespace mwr {

using std::string;
using std::vector;
using std::optional;

bool file_exists(const string& file);
bool directory_exists(const string& file);

bool is_absolute_path(const string& path);
bool is_relative_path(const string& path);
bool is_path_equal(const string& p1, const string& p2);

string dirname(const string& path);
string filename(const string& path);
string filename_noext(const string& path);

string curr_dir();
string temp_dir();

string progname();
string username();

optional<string> getenv(const string& name);
void setenv(const string& name, const string& value);

int getpid();

size_t get_page_size();

inline bool is_page_aligned(uintptr_t addr) {
    return (addr & (get_page_size() - 1)) == 0;
}

template <typename T>
inline bool is_page_aligned(T* addr) {
    return is_page_aligned((uintptr_t)addr);
}

constexpr int STDIN_FDNO = 0;
constexpr int STDOUT_FDNO = 1;
constexpr int STDERR_FDNO = 2;

#if defined(MWR_WINDOWS)
constexpr size_t OS_MAX_PATH = 260;
#elif defined(MWR_LINUX)
constexpr size_t OS_MAX_PATH = 4096;
#elif defined(MWR_OSX)
constexpr size_t OS_MAX_PATH = 1024;
#endif

int fd_open(const string& path, const string& mode, int perms = 0644);
int fd_open(const string& path, int mode, int perms = 0644);
void fd_close(int fd);

size_t fd_peek(int fd, time_t timeout_ms = 0);
size_t fd_read(int fd, void* buffer, size_t buflen);
size_t fd_write(int fd, const void* buffer, size_t buflen);

size_t fd_seek(int fd, size_t pos);
size_t fd_seek_cur(int fd, off_t pos);
size_t fd_seek_end(int fd, off_t pos);

int fd_dup(int fd);
int fd_pipe(int fds[2]);

double timestamp();
u64 timestamp_ms();
u64 timestamp_us();
u64 timestamp_ns();

inline void cpu_yield() {
#if defined(MWR_MSVC)
    _mm_pause();
#elif defined(MWR_X86_64)
    asm volatile("pause" ::: "memory");
#elif defined(MWR_ARM64)
    asm volatile("yield" ::: "memory");
#endif
}

bool fill_random(void* buffer, size_t bufsz);

} // namespace mwr

#endif
