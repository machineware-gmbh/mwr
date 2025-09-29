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

#if defined(MWR_LINUX) || defined(MWR_MACOS)
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif

#if defined(MWR_MSVC) || defined(MWR_MINGW)
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
#ifdef MWR_WINDOWS
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
#ifdef MWR_WINDOWS
        DWORD attr;
#else
        termios attr;
#endif
        bool restore;
    };

    int m_fd;
    std::stack<ttystate> m_stack;

#ifdef MWR_WINDOWS
    DWORD load() const {
        DWORD mode;
        HANDLE console = (HANDLE)_get_osfhandle(m_fd);
        if (!GetConsoleMode(console, &mode))
            MWR_ERROR("failed to get console attributes");
        return mode;
    }

    void save(DWORD mode) const {
        HANDLE console = (HANDLE)_get_osfhandle(m_fd);
        if (!SetConsoleMode(console, mode))
            MWR_ERROR("failed to set console attributes");
    }
#else
    termios load() const {
        termios attr;
        if (tcgetattr(m_fd, &attr))
            MWR_ERROR("failed to get termios attributes: %s", strerror(errno));
        return attr;
    }

    void save(const termios& attr) {
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
#ifdef MWR_WINDOWS
                HANDLE console = (HANDLE)_get_osfhandle(m_fd);
                SetConsoleMode(console, state.attr);
#else
                tcsetattr(m_fd, TCSAFLUSH, &state.attr);
#endif
            }
        }
    }

#ifdef MWR_WINDOWS
    bool is_vt100() const {
        return load() & (ENABLE_VIRTUAL_TERMINAL_INPUT |
                         ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#else
    bool is_echo() const { return load().c_lflag & ECHO; }
    bool is_isig() const { return load().c_lflag & ISIG; }
    bool is_vt100() const { return !is_echo() && !is_isig(); }
#endif

    void setup_vt100() {
        auto attr = load();
#ifdef MWR_WINDOWS
        if (m_fd == STDIN_FDNO) {
            attr &= ~ENABLE_ECHO_INPUT;
            attr &= ~ENABLE_LINE_INPUT;
            attr &= ~ENABLE_MOUSE_INPUT;
            attr &= ~ENABLE_PROCESSED_INPUT;
            attr |= ENABLE_VIRTUAL_TERMINAL_INPUT;
        } else {
            attr |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        }
#else
        attr.c_lflag &= ~ICANON;
        attr.c_lflag &= ~ECHONL;
        attr.c_lflag &= ~ECHO;
        attr.c_lflag &= ~ISIG;
        attr.c_cc[VMIN] = 1;
        attr.c_cc[VTIME] = 0;
#endif
        save(attr);

#ifdef MWR_WINDOWS
        if (m_fd == STDIN_FDNO)
            SetConsoleCP(CP_UTF8);
        else
            SetConsoleOutputCP(CP_UTF8);
#endif
    }

    void push(bool restore) {
        ttystate state;
        state.attr = load();
        state.restore = restore;
        m_stack.push(state);
    }

    void pop() {
        MWR_ERROR_ON(m_stack.empty(), "no tty state on stack");
        save(m_stack.top().attr);
        m_stack.pop();
    }

    static tty& lookup(int fd) {
        static unordered_map<int, std::shared_ptr<tty>> ttys;
        auto& res = ttys[fd];
        if (!res)
            res = std::make_shared<tty>(fd);
        return *res;
    }
};

bool tty_is_vt100(int fd) {
    return tty::lookup(fd).is_vt100();
}

void tty_push(int fd, bool restore) {
    tty::lookup(fd).push(restore);
}

void tty_pop(int fd) {
    tty::lookup(fd).pop();
}

void tty_setup_vt100(int fd) {
    tty::lookup(fd).setup_vt100();
}

} // namespace mwr
