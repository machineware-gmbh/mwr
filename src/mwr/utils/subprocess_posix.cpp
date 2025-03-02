/******************************************************************************
 *                                                                            *
 * Copyright (C) 2025 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/utils/subprocess.h"

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace mwr {

subprocess::subprocess():
    m_pid(-1), m_stdin(-1), m_stdout(-1), m_stderr(-1), env() {
}

subprocess::~subprocess() {
    terminate();
}

static void set_o_nonblock(int fd, bool set) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (set)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

bool subprocess::run(const string& path, const vector<string>& args) {
    int stdin_pipe[2];
    int stdout_pipe[2];
    int stderr_pipe[2];

    if (pipe(stdin_pipe) < 0)
        MWR_ERROR("error creating pipe: %s", strerror(errno));
    if (pipe(stdout_pipe) < 0)
        MWR_ERROR("error creating pipe: %s", strerror(errno));
    if (pipe(stderr_pipe) < 0)
        MWR_ERROR("error creating pipe: %s", strerror(errno));

    int pid = fork();
    if (pid < 0)
        return false;

    if (pid == 0) { // child process
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);

        vector<char*> argv;
        argv.push_back(const_cast<char*>(path.c_str()));
        for (auto& arg : args)
            argv.push_back(const_cast<char*>(arg.c_str()));
        argv.push_back(nullptr);

        vector<char*> envp;
        if (env.empty()) {
            execv(path.c_str(), argv.data());
        } else {
            vector<string> env_strings(env.size());
            for (auto& [name, value] : env)
                env_strings.push_back(strcat(name, "=", value));
            for (auto& s : env_strings)
                envp.push_back(const_cast<char*>(s.c_str()));
            envp.push_back(nullptr);
            execve(path.c_str(), argv.data(), envp.data());
        }

        _exit(1);
    } else { // parent process
        close(stdout_pipe[1]);
        close(stdin_pipe[0]);
        m_stdin = stdin_pipe[1];
        m_stdout = stdout_pipe[0];
        m_stderr = stderr_pipe[0];
        m_pid = pid;
        return true;
    }
}

bool subprocess::terminate() {
    if (m_pid < 0)
        return false;

    if (kill(m_pid, SIGTERM))
        return false;

    int status;
    waitpid(m_pid, &status, 0);

    m_stdin = -1;
    m_stdout = -1;
    m_stderr = -1;
    m_pid = -1;

    return true;
}

bool subprocess::write(const string& s) {
    if (m_stdin < 0)
        return false;
    if (s.empty())
        return true;

    ssize_t n = ::write(m_stdin, s.data(), s.length());
    return n < 0 ? false : (size_t)n == s.length();
}

bool subprocess::read(char* buf, size_t buflen, bool use_stderr) {
    int fd = use_stderr ? m_stderr : m_stdout;
    if (fd < 0)
        return false;

    set_o_nonblock(fd, false);

    size_t nread = 0;
    while (nread < buflen) {
        ssize_t res = ::read(fd, buf + nread, buflen - nread);
        if (res < 0)
            return false;
        if (res == 0)
            return false; // EOF
        nread += (ssize_t)res;
    }

    return true;
}

string subprocess::read(bool use_stderr) {
    int fd = use_stderr ? m_stderr : m_stdout;
    if (fd < 0)
        return "";

    set_o_nonblock(fd, true);

    char buf[1024] = {};
    ssize_t n = ::read(fd, buf, sizeof(buf) - 1);
    return n < 0 ? "" : buf;
}

} // namespace mwr
