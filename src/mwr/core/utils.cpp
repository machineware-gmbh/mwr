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
#include "mwr/stl/strings.h"

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <fcntl.h>
#include <chrono>
#include <filesystem>

namespace chrono = std::chrono;
namespace fs = std::filesystem;

#ifdef MWR_LINUX
#include <unistd.h>
#include <execinfo.h>
#include <cxxabi.h>
#endif

#ifdef MWR_WINDOWS
#include <Windows.h>
#include <DbgHelp.h>
#include <io.h>
#endif

#ifdef MWR_MACOS
#include <unistd.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <mach-o/dyld.h>
#endif

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

bool is_absolute_path(const string& path) {
    return fs::path(path).is_absolute();
}

bool is_relative_path(const string& path) {
    return fs::path(path).is_relative();
}

bool is_path_equal(const string& p1, const string& p2) {
    return fs::path(p1) == fs::path(p2);
}

string dirname(const string& path) {
    fs::path full(path);
    string result = full.parent_path().string();
    return result.empty() ? "." : result;
}

string filename(const string& path) {
    fs::path full(path);
    return full.filename().string();
}

string filename_noext(const string& path) {
    const string name = filename(path);
    size_t i = name.rfind('.', path.length());
    return (i == string::npos) ? name : name.substr(0, i);
}

string curr_dir() {
    auto path = std::filesystem::current_path();
    return path.string();
}

string temp_dir() {
#if defined(MWR_LINUX)
    return "/tmp";
#elif defined(MWR_MACOS)
    return "/private/tmp";
#elif defined(MWR_WINDOWS)
    TCHAR path[MAX_PATH] = {};
    if (GetTempPath(MAX_PATH, path) == 0)
        MWR_ERROR("failed to retrieve temp dir");
    return string(path);
#endif
}

string progname() {
#if defined(MWR_LINUX)
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);

    if (len == -1)
        return "unknown";

    path[len] = '\0';
    return path;
#elif defined(MWR_WINDOWS)
    TCHAR path[MAX_PATH] = {};
    if (GetModuleFileName(NULL, path, MAX_PATH) == 0)
        MWR_ERROR("failed to get the program name");
    return string(path);
#elif defined(MWR_MACOS)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size))
        return "unknown";

    return path;
#endif
}

string username() {
#if defined(MWR_WINDOWS)
    TCHAR name[MAX_PATH] = {};
    DWORD namelen = sizeof(name);
    if (GetUserName(name, &namelen))
        return string(name);
#else
    char uname[256] = {};
    if (getlogin_r(uname, sizeof(uname) - 1) == 0)
        return uname;

    auto envuser = getenv("USER");
    if (envuser)
        return envuser.value();
#endif
    return "unknown";
}

optional<string> getenv(const string& env) {
#if defined(MWR_MSVC)
    DWORD n = GetEnvironmentVariable(env.c_str(), NULL, 0);
    if (n == 0)
        return optional<string>();

    string var(n - 1, 0);
    GetEnvironmentVariableA(env.c_str(), var.data(), n);
    return var;
#else
    const char* val = std::getenv(env.c_str());
    if (val == nullptr)
        return optional<string>();
    return val;
#endif
}

void setenv(const string& name, const string& value) {
#if defined(MWR_MSVC)
    auto err = _putenv_s(name.c_str(), value.c_str());
    if (err != 0)
        MWR_ERROR("failed to set %s (%d)", name.c_str(), err);
#else
    if (::setenv(name.c_str(), value.c_str(), 1))
        MWR_ERROR("failed to set %s (%s)", name.c_str(), strerror(errno));
#endif
}

int getpid() {
#if defined(MWR_MSVC)
    return (int)GetCurrentProcessId();
#else
    return (int)::getpid();
#endif
}

static size_t os_get_page_size() {
#if defined(MWR_WINDOWS)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
#else
    return sysconf(_SC_PAGE_SIZE);
#endif
}

size_t get_page_size() {
    static size_t cached_size = os_get_page_size();
    return cached_size;
}

vector<string> backtrace(size_t frames, size_t skip) {
    vector<string> sv;

#if defined(MWR_LINUX) || defined(MWR_MACOS)

    vector<void*> symbols(frames + skip);
    size_t size = (size_t)::backtrace(symbols.data(), symbols.size());
    if (size <= skip)
        return sv;

    sv.resize(size - skip);

    size_t dmbufsz = 256;
    char* dmbuf = (char*)malloc(dmbufsz);
    char** names = ::backtrace_symbols(symbols.data(), symbols.size());
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

#elif defined(MWR_WINDOWS)

    void* symbols[256];
    if (frames > MWR_ARRAY_SIZE(symbols))
        frames = MWR_ARRAY_SIZE(symbols);
    size_t size = CaptureStackBackTrace((DWORD)skip, (DWORD)frames, symbols,
                                        NULL);

    SymInitialize(GetCurrentProcess(), NULL, TRUE);
    for (size_t i = 0; i < size; i++) {
        DWORD64 address = (DWORD64)symbols[i];
        DWORD64 offset = 0;

        char buffer[sizeof(SYMBOL_INFO) + MAX_PATH];
        SYMBOL_INFO* symbol = (SYMBOL_INFO*)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_PATH;

        if (SymFromAddr(GetCurrentProcess(), address, &offset, symbol))
            sv.push_back(mkstr("%s+0x%x", symbol->Name, offset));
        else
            sv.push_back(mkstr("<unknown> [%p]", symbols[i]));
    }

#endif

    return sv;
}

size_t max_backtrace_length = 16;

#if defined(MWR_MSVC)
static LPTOP_LEVEL_EXCEPTION_FILTER prev_handler;
static LONG WINAPI handle_exception(EXCEPTION_POINTERS* info) {
    if (info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        fprintf(stderr, "Backtrace\n");
        auto symbols = backtrace(max_backtrace_length, 2);
        for (size_t i = symbols.size(); i > 0; i--)
            fprintf(stderr, "# %2zu: %s\n", i - 1, symbols[i - 1].c_str());
        fprintf(stderr,
                "EXCEPTION_ACCESS_VIOLATION while accessing memory at %p\n",
                (void*)info->ExceptionRecord->ExceptionInformation[1]);
        fflush(stderr);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    if (prev_handler)
        return prev_handler(info);
    else
        return EXCEPTION_CONTINUE_SEARCH;
}

void report_segfaults() {
    prev_handler = SetUnhandledExceptionFilter(handle_exception);
}

#else
static struct sigaction oldact;
static void handle_segfault(int sig, siginfo_t* info, void* context) {
    fprintf(stderr, "Backtrace\n");
    auto symbols = backtrace(max_backtrace_length, 2);
    for (size_t i = symbols.size(); i > 0; i--)
        fprintf(stderr, "# %2zu: %s\n", i - 1, symbols[i - 1].c_str());
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
#endif

int fd_open(const string& path, const string& mode, int perms) {
#ifdef MWR_MSVC
    perms &= ~077;

    int flags = 0;
    if (mode == "r")
        flags = _O_RDONLY;
    else if (mode == "rb")
        flags = _O_RDONLY | _O_BINARY;
    else if (mode == "w")
        flags = _O_WRONLY | _O_CREAT | _O_TRUNC;
    else if (mode == "wb")
        flags = _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY;
    else if (mode == "a")
        flags = _O_WRONLY | _O_CREAT | _O_APPEND;
    else if (mode == "ab")
        flags = _O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY;
    else if (mode == "r+")
        flags = _O_RDWR;
    else if (mode == "rb+")
        flags = _O_RDWR | _O_BINARY;
    else if (mode == "w+")
        flags = _O_RDWR | _O_CREAT | _O_TRUNC;
    else if (mode == "wb+")
        flags = _O_RDWR | _O_CREAT | _O_TRUNC | _O_BINARY;
    else if (mode == "a+")
        flags = _O_RDWR | _O_CREAT | _O_APPEND;
    else if (mode == "ab+")
        flags = _O_RDWR | _O_CREAT | _O_APPEND | _O_BINARY;
    else
        MWR_ERROR("invalid openmode '%s'", mode.c_str());
#else
    int flags = 0;
    if (mode == "r" || mode == "rb")
        flags = O_RDONLY;
    else if (mode == "w" || mode == "wb")
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    else if (mode == "a" || mode == "ab")
        flags = O_WRONLY | O_CREAT | O_APPEND;
    else if (mode == "r+" || mode == "rb+")
        flags = O_RDWR;
    else if (mode == "w+" || mode == "wb+")
        flags = O_RDWR | O_CREAT | O_TRUNC;
    else if (mode == "a+" || mode == "ab+")
        flags = O_RDWR | O_CREAT | O_APPEND;
    else
        MWR_ERROR("invalid openmode '%s'", mode.c_str());
#endif
    return fd_open(path, flags, perms);
}

int fd_open(const string& path, int mode, int perms) {
#ifdef MWR_MSVC
    perms &= ~077;
    int fd = -1;
    errno_t err = _sopen_s(&fd, path.c_str(), mode, _SH_DENYNO, perms);
    if (err) {
        errno = err;
        return -1;
    }

    return fd;
#else
    return open(path.c_str(), mode, perms);
#endif
}

void fd_close(int fd) {
    if (fd < 0)
        return;

#ifdef MWR_MSVC
    _close(fd);
#else
    close(fd);
#endif
}

size_t fd_peek(int fd, time_t timeoutms) {
    if (fd < 0)
        return 0;

#if defined(MWR_MSVC)
    HANDLE handle = (HANDLE)_get_osfhandle(fd);
    if (handle == INVALID_HANDLE_VALUE)
        MWR_ERROR("invalid file descriptor: %d", fd);

    switch (GetFileType(handle)) {
    case FILE_TYPE_CHAR: {
        if (WaitForSingleObject(handle, (DWORD)timeoutms) == WAIT_TIMEOUT)
            return 0;

        DWORD nevents = 0;
        if (GetNumberOfConsoleInputEvents(handle, &nevents))
            return nevents;

        return 0;
    }

    case FILE_TYPE_PIPE: {
        DWORD avail = 0;
        if (PeekNamedPipe(handle, NULL, 0, NULL, &avail, NULL) && avail > 0)
            return avail;

        Sleep((DWORD)timeoutms);
        return 0;
    }

    case FILE_TYPE_DISK: {
        LARGE_INTEGER size, pos;
        if (!GetFileSizeEx(handle, &size))
            MWR_ERROR("failed to get size of fd %d", fd);
        if (!SetFilePointerEx(handle, { 0 }, &pos, FILE_CURRENT))
            MWR_ERROR("failed to possition of fd %d", fd);
        if (pos.QuadPart < size.QuadPart)
            return size.QuadPart - pos.QuadPart - 1;
        return 0;
    }

    default:
        return 0;
    }
#else
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
#endif
}

#ifdef MWR_MSVC
// workaround for https://github.com/microsoft/terminal/issues/16299
static size_t msvc_read_console(u8* buf, size_t len) {
    static HANDLE console = GetStdHandle(STD_INPUT_HANDLE);
    static char data;

    if (data == 1) { // ctrl-a
        data = getchar();
        *buf = data;
        return 1;
    }

    DWORD nevents = 0;
    INPUT_RECORD input;
    ReadConsoleInput(console, &input, 1, &nevents);
    if (nevents == 0 || input.EventType != KEY_EVENT ||
        !input.Event.KeyEvent.bKeyDown) {
        return 0;
    }

    data = input.Event.KeyEvent.uChar.AsciiChar;
    *buf = data;
    return 1;
}
#endif

size_t fd_read(int fd, void* buffer, size_t buflen) {
    if (fd < 0 || buffer == nullptr || buflen == 0)
        return 0;

    u8* ptr = reinterpret_cast<u8*>(buffer);

    long long len;
    size_t numread = 0;

#ifdef MWR_MSVC
    if (fd == STDIN_FDNO && _isatty(fd))
        return msvc_read_console(ptr, buflen);
#endif

    while (numread < buflen) {
        do {
#ifdef MWR_MSVC
            size_t n = min(buflen - numread, U32_MAX);
            len = _read(fd, ptr + numread, (u32)n);
#else
            len = read(fd, ptr + numread, buflen - numread);
#endif
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

    long long len;
    size_t written = 0;

    while (written < buflen) {
        do {
#ifdef MWR_MSVC
            size_t n = min(buflen - written, U32_MAX);
            len = _write(fd, ptr + written, (u32)n);
#else
            len = write(fd, ptr + written, buflen - written);
#endif
        } while (len < 0 && errno == EINTR);

        if (len <= 0)
            return written;

        written += len;
    }

    return written;
}

size_t fd_seek(int fd, size_t pos) {
#ifdef MWR_MSVC
    return (size_t)_lseeki64(fd, pos, SEEK_SET);
#else
    return (size_t)lseek(fd, pos, SEEK_SET);
#endif
}

size_t fd_seek_cur(int fd, off_t pos) {
#ifdef MWR_MSVC
    return (size_t)_lseeki64(fd, pos, SEEK_CUR);
#else
    return (size_t)lseek(fd, pos, SEEK_CUR);
#endif
}

size_t fd_seek_end(int fd, off_t pos) {
#ifdef MWR_MSVC
    return (size_t)_lseeki64(fd, pos, SEEK_END);
#else
    return (size_t)lseek(fd, pos, SEEK_END);
#endif
}

int fd_dup(int fd) {
#ifdef MWR_MSVC
    return _dup(fd);
#else
    return dup(fd);
#endif
}

int fd_pipe(int fds[2]) {
#ifdef MWR_MSVC
    return _pipe(fds, 2, _O_TEXT);
#else
    return pipe(fds);
#endif
}

using timestamp_clock = chrono::steady_clock;
static auto g_start = timestamp_clock::now();

double timestamp() {
    chrono::duration<double> delta = timestamp_clock::now() - g_start;
    return delta.count();
}

u64 timestamp_ms() {
    auto delta = timestamp_clock::now() - g_start;
    return chrono::duration_cast<chrono::milliseconds>(delta).count();
}

u64 timestamp_us() {
    auto delta = timestamp_clock::now() - g_start;
    return chrono::duration_cast<chrono::microseconds>(delta).count();
}

u64 timestamp_ns() {
    auto delta = timestamp_clock::now() - g_start;
    return chrono::duration_cast<chrono::nanoseconds>(delta).count();
}

bool fill_random(void* buffer, size_t bufsz) {
#if defined(MWR_WINDOWS)

    HCRYPTPROV provider;
    if (!CryptAcquireContext(&provider, NULL, NULL, PROV_RSA_FULL,
                             CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        return false;
    }

    BYTE* ptr = (BYTE*)buffer;
    size_t len = 0;

    while (len < bufsz) {
        DWORD n = (DWORD)min(bufsz - len, ~0u);
        if (!CryptGenRandom(provider, n, ptr))
            break;

        len += n;
        ptr += n;
    }

    CryptReleaseContext(provider, 0);
    return len == bufsz;

#else

    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0)
        return false;

    bool ok = fd_read(fd, buffer, bufsz);
    fd_close(fd);
    return ok;

#endif
}

} // namespace mwr
