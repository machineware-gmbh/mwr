/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/core/utils.h"
#include "mwr/core/report.h"
#include "mwr/core/compiler.h"
#include "mwr/stl/strings.h"

#include <string.h>
#include <signal.h>
#include <limits.h>

#ifdef __linux__
#include <unistd.h>
#include <execinfo.h>
#include <cxxabi.h>
#endif

#include <filesystem>

namespace fs = std::filesystem;

namespace mwr {

static fs::path resolve(const string& file) {
    size_t limit = 10;
    fs::path path(file);
    while (fs::is_symlink(path) && limit--) {
        fs::path link = fs::read_symlink(path);
        path = (link.is_relative() ? path.parent_path() : "") / link;
    }

    return path;
}

bool file_exists(const string& file) {
    try {
        return fs::is_regular_file(resolve(file));
    } catch (...) {
        return false;
    }
}

bool directory_exists(const string& dir) {
    try {
        return fs::is_directory(resolve(dir));
    } catch (...) {
        return false;
    }
}

string dirname(const string& path) {
#ifdef _WIN32
    const char separator = '\\';
#else
    const char separator = '/';
#endif
    size_t i = path.rfind(separator, path.length());
    return (i == string::npos) ? "." : path.substr(0, i);
}

string filename(const string& path) {
#ifdef _WIN32
    const char separator = '\\';
#else
    const char separator = '/';
#endif
    size_t i = path.rfind(separator, path.length());
    return (i == string::npos) ? path : path.substr(i + 1);
}

string filename_noext(const string& path) {
    const string name = filename(path);
    size_t i = name.rfind('.', path.length());
    return (i == string::npos) ? name : name.substr(0, i);
}

string curr_dir() {
#ifdef __linux__
    char path[PATH_MAX];
    if (getcwd(path, sizeof(path)) != path)
        MWR_ERROR("cannot read current directory: %s", strerror(errno));
    return string(path);
#else
    return "";
#endif
}

string temp_dir() {
#ifdef _WIN32
    // ToDo: implement tempdir for windows
#else
    return "/tmp/";
#endif
}

string progname() {
#ifdef __linux__
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);

    if (len == -1)
        return "unknown";

    path[len] = '\0';
    return path;
#else
    return "unknown";
#endif
}

string username() {
#ifdef __linux__
    char uname[256] = {};
    if (getlogin_r(uname, sizeof(uname) - 1) == 0)
        return uname;

    const char* envuser = getenv("USER");
    if (envuser)
        return envuser;
#endif

    return "unknown";
}

vector<string> backtrace(size_t frames, size_t skip) {
    vector<string> sv;
 #ifdef __linux__
    void* symbols[frames + skip];
    size_t size = (size_t)::backtrace(symbols, frames + skip);
    if (size <= skip)
        return sv;

    sv.resize(size - skip);

    size_t dmbufsz = 256;
    char* dmbuf = (char*)malloc(dmbufsz);
    char** names = ::backtrace_symbols(symbols, size);
    for (size_t i = skip; i < size; i++) {
        char *func = nullptr, *offset = nullptr, *end = nullptr;
        for (char* ptr = names[i]; *ptr != '\0'; ptr++) {
            if (*ptr == '(')
                func = ptr++;
            else if (*ptr == '+')
                offset = ptr++;
            else if (*ptr == ')') {
                end = ptr++;
                break;
            }
        }

        if (!func || !offset || !end) {
            sv[i - skip] = mkstr("<unknown> [%p]", symbols[i]);
            continue;
        }

        *func++ = '\0';
        *offset++ = '\0';
        *end = '\0';

        sv[i - skip] = string(func) + "+" + string(offset);

        int status = 0;
        char* res = abi::__cxa_demangle(func, dmbuf, &dmbufsz, &status);
        if (status == 0)
            sv[i - skip] = string(dmbuf = res) + "+" + string(offset);
    }

    free(names);
    free(dmbuf);
#endif
    return sv;
}

size_t max_backtrace_length = 16;
static struct sigaction oldact;

static void handle_segfault(int sig, siginfo_t* info, void* context) {
    fprintf(stderr, "Backtrace\n");
    auto symbols = backtrace(max_backtrace_length, 2);
    for (size_t i = symbols.size() - 1; i < symbols.size(); i--)
        fprintf(stderr, "# %2zu: %s\n", i, symbols[i].c_str());
    fprintf(stderr,
            "Caught signal %d (%s) while accessing memory at location %p\n",
            sig, strsignal(sig), info->si_addr);
    fflush(stderr);

    if ((oldact.sa_flags & SA_SIGINFO) && (oldact.sa_sigaction != NULL)) {
        (*oldact.sa_sigaction)(sig, info, context);
        return;
    }

    if (oldact.sa_handler != NULL) {
        (*oldact.sa_handler)(sig);
        return;
    }

    // If there is no other handler to call, reset the handler back to the
    // default signal handler and then re-raise the signal again.
    signal(sig, SIG_DFL);
    raise(sig);
}

void report_segfaults() {
    struct sigaction newact;
    memset(&newact, 0, sizeof(newact));
    memset(&oldact, 0, sizeof(oldact));

    sigemptyset(&newact.sa_mask);
    newact.sa_sigaction = &handle_segfault;
    newact.sa_flags = SA_SIGINFO;

    if (::sigaction(SIGSEGV, &newact, &oldact) < 0)
        MWR_ERROR("failed to install SIGSEGV signal handler");
}

size_t fd_peek(int fd, time_t timeoutms) {
    if (fd < 0)
        return 0;

    fd_set in, out, err;
    struct timeval timeout;

    FD_ZERO(&in);
    FD_SET(fd, &in);
    FD_ZERO(&out);
    FD_ZERO(&err);

    timeout.tv_sec = (timeoutms / 1000ull);
    timeout.tv_usec = (timeoutms % 1000ull) * 1000ull;

    struct timeval* ptimeout = ~timeoutms ? &timeout : nullptr;
    int ret = select(fd + 1, &in, &out, &err, ptimeout);
    return ret > 0 ? 1 : 0;
}

size_t fd_read(int fd, void* buffer, size_t buflen) {
    if (fd < 0 || buffer == nullptr || buflen == 0)
        return 0;

    u8* ptr = reinterpret_cast<u8*>(buffer);

    ssize_t len;
    size_t numread = 0;

    while (numread < buflen) {
        do {
            len = ::read(fd, ptr + numread, buflen - numread);
        } while (len < 0 && errno == EINTR);

        if (len <= 0)
            return numread;

        numread += len;
    }

    return numread;
}

size_t fd_write(int fd, const void* buffer, size_t buflen) {
    if (fd < 0 || buffer == nullptr || buflen == 0)
        return 0;

    const u8* ptr = reinterpret_cast<const u8*>(buffer);

    ssize_t len;
    size_t written = 0;

    while (written < buflen) {
        do {
            len = ::write(fd, ptr + written, buflen - written);
        } while (len < 0 && errno == EINTR);

        if (len <= 0)
            return written;

        written += len;
    }

    return written;
}

size_t fd_seek(int fd, size_t pos) {
    return (size_t)lseek(fd, pos, SEEK_SET);
}

size_t fd_seek_cur(int fd, off_t pos) {
    return (size_t)lseek(fd, pos, SEEK_CUR);
}

size_t fd_seek_end(int fd, off_t pos) {
    return (size_t)lseek(fd, pos, SEEK_END);
}

double timestamp() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec + tp.tv_nsec * 1e-9;
}

u64 timestamp_ms() {
    struct timespec tp = {};
    if (clock_gettime(CLOCK_MONOTONIC, &tp))
        MWR_ERROR("cannot read clock: %s (%d)", strerror(errno), errno);
    return (u64)tp.tv_sec * 1000ull + (u64)tp.tv_nsec / 1000000ull;
}

u64 timestamp_us() {
    struct timespec tp = {};
    if (clock_gettime(CLOCK_MONOTONIC, &tp))
        MWR_ERROR("cannot read clock: %s (%d)", strerror(errno), errno);
    return (u64)tp.tv_sec * 1000000ull + (u64)tp.tv_nsec / 1000ull;
}

u64 timestamp_ns() {
    struct timespec tp = {};
    if (clock_gettime(CLOCK_MONOTONIC, &tp))
        MWR_ERROR("cannot read clock: %s (%d)", strerror(errno), errno);
    return (u64)tp.tv_sec * 1000000000ull + (u64)tp.tv_nsec;
}

} // namespace mwr
