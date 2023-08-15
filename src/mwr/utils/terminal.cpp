/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/utils/terminal.h"

#include <stack>
#include <memory>

#include <errno.h>
#include <string.h>

#if defined(MWR_LINUX)
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif

#if defined(MWR_MSVC)
#include <Windows.h>
#include <io.h>
#endif

namespace mwr {

int new_tty() {
#if defined(MWR_WINDOWS)
    errno = ENOSYS;
    return -1;
#else
    static int master = posix_openpt(O_RDWR | O_NOCTTY);
    MWR_ERROR_ON(master < 0, "posix_openpt() failed: %s", strerror(errno));

    char path[256];
    if (grantpt(master) || unlockpt(master) ||
        ptsname_r(master, path, sizeof(path))) {
        MWR_ERROR("pty setup failed: %s", strerror(errno));
    }

    return open(path, O_RDWR | O_NOCTTY);
#endif
}

bool is_tty(int fd) {
#ifdef MWR_MSVC
    return fd >= 0 && _isatty(fd);
#else
    termios attr;
    return tcgetattr(fd, &attr) == 0;
#endif
}

class tty
{
private:
    struct ttystate {
#ifdef MWR_MSVC
        DWORD attr;
#else
        termios attr;
#endif
        bool restore;
    };

    int m_fd;
    std::stack<ttystate> m_stack;

#ifdef MWR_MSVC
    DWORD get() const { 
        DWORD mode;
        HANDLE console = (HANDLE)_get_osfhandle(m_fd);
        if (!GetConsoleMode(console, &mode))
            MWR_ERROR("failed to get console attributes");
        return mode;
    }

    void set(DWORD mode) const {
        HANDLE console = (HANDLE)_get_osfhandle(m_fd);
        if (!SetConsoleMode(console, mode))
            MWR_ERROR("failed to set console attributes");
    }
#else
    termios get() const {
        termios attr;
        if (tcgetattr(m_fd, &attr))
            MWR_ERROR("failed to get termios attributes: %s", strerror(errno));
        return attr;
    }

    void set(const termios& attr) {
        if (tcsetattr(m_fd, TCSAFLUSH, &attr))
            MWR_ERROR("failed to get termios attributes: %s", strerror(errno));
    }
#endif

public:
    tty(int fd): m_fd(fd) { MWR_ERROR_ON(!is_tty(fd), "not a tty: %d", fd); }

    ~tty() {
        while (!m_stack.empty()) {
            auto state = m_stack.top();
            m_stack.pop();

            // try to restore the tty state. its okay if it fails, maybe the
            // corresponding fd has already been closed.
            if (state.restore) {
#ifdef MWR_MSVC
                HANDLE console = (HANDLE)_get_osfhandle(m_fd);
                SetConsoleMode(console, state.attr);
#else
                tcsetattr(m_fd, TCSAFLUSH, &state.attr);
#endif
            }
        }
    }

#ifdef MWR_MSVC
    bool is_echo() const { return get() & ENABLE_ECHO_INPUT; }
    bool is_isig() const { return get() & ENABLE_PROCESSED_INPUT; }
#else
    bool is_echo() const { return get().c_lflag & ECHO; }
    bool is_isig() const { return get().c_lflag & ISIG; }
#endif

    void set(bool echo, bool isig) {
#ifdef MWR_MSVC
        DWORD attr = get();
        attr &= ~(ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
        if (echo)
            attr |= ENABLE_ECHO_INPUT;
        if (isig)
            attr |= ENABLE_PROCESSED_INPUT;
#else
        termios attr = get();
        set_bit<ICANON>(attr.c_lflag, echo);
        set_bit<ECHONL>(attr.c_lflag, echo);
        set_bit<ECHO>(attr.c_lflag, echo);
        set_bit<ISIG>(attr.c_lflag, isig);
        attr.c_cc[VMIN] = 1;
        attr.c_cc[VTIME] = 0;
#endif
        set(attr);
    }

    void push(bool restore) {
        ttystate state;
        state.attr = get();
        state.restore = restore;
        m_stack.push(state);
    }

    void pop() {
        MWR_ERROR_ON(m_stack.empty(), "no tty state on stack");
        set(m_stack.top().attr);
        m_stack.pop();
    }

    static tty& get(int fd) {
        static unordered_map<int, std::shared_ptr<tty>> ttys;
        auto& res = ttys[fd];
        if (!res)
            res = std::make_shared<tty>(fd);
        return *res;
    }
};

bool tty_is_echo(int fd) {
    return tty::get(fd).is_echo();
}

bool tty_is_isig(int fd) {
    return tty::get(fd).is_isig();
}

void tty_push(int fd, bool restore) {
    tty::get(fd).push(restore);
}

void tty_pop(int fd) {
    tty::get(fd).pop();
}

void tty_set(int fd, bool echo, bool signal) {
    tty::get(fd).set(echo, signal);
}

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
