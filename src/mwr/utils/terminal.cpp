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
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

namespace mwr {

int new_tty() {
    static int master = posix_openpt(O_RDWR | O_NOCTTY);
    MWR_ERROR_ON(master < 0, "posix_openpt() failed: %s", strerror(errno));

    if (grantpt(master)) {
        close(master);
        MWR_ERROR("grantpt() failed: %s", strerror(errno));
    }

    if (unlockpt(master)) {
        close(master);
        MWR_ERROR("unlockpt() failed: %s", strerror(errno));
    }

    char path[256];
    if (ptsname_r(master, path, sizeof(path))) {
        close(master);
        MWR_ERROR("ptsname_r() failed: %s", path);
    }

    return open(path, O_RDWR | O_NOCTTY);
}

bool is_tty(int fd) {
    termios attr;
    printf("fd %d is%s a tty\n", fd, tcgetattr(fd, &attr) ? " not" : "");
    return tcgetattr(fd, &attr) == 0;
}

class tty
{
private:
    struct ttystate {
        termios attr;
        bool restore;
    };

    int m_fd;
    std::stack<ttystate> m_stack;

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

public:
    tty(int fd): m_fd(fd) { MWR_ERROR_ON(!isatty(fd), "not a tty: %d", fd); }

    ~tty() {
        while (!m_stack.empty()) {
            auto state = m_stack.top();
            m_stack.pop();
            if (state.restore)
                set(state.attr);
        }
    }

    bool is_echo() const { return get().c_lflag & ECHO; }
    bool is_isig() const { return get().c_lflag & ISIG; }

    void set(bool echo, bool isig) {
        termios attr = get();
        set_bit<ICANON>(attr.c_lflag, echo);
        set_bit<ECHONL>(attr.c_lflag, echo);
        set_bit<ECHO>(attr.c_lflag, echo);
        set_bit<ISIG>(attr.c_lflag, isig);
        attr.c_cc[VMIN] = 1;
        attr.c_cc[VTIME] = 0;
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
