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

#include <Windows.h>

namespace mwr {

subprocess::subprocess():
    m_handle(NULL),
    m_stdin(NULL),
    m_stdout(NULL),
    m_stderr(NULL),
    m_pid(),
    env() {
}

subprocess::~subprocess() {
    terminate();
}

bool subprocess::run(const string& path, const vector<string>& args) {
    SECURITY_ATTRIBUTES sa_attr = { sizeof(SECURITY_ATTRIBUTES), nullptr,
                                    TRUE };

    HANDLE stdin_read = nullptr, stdin_write = nullptr;
    if (!CreatePipe(&stdin_read, &stdin_write, &sa_attr, 0))
        return false;
    if (!SetHandleInformation(stdin_write, HANDLE_FLAG_INHERIT, 0))
        return false;

    HANDLE stdout_read = nullptr, stdout_write = nullptr;
    if (!CreatePipe(&stdout_read, &stdout_write, &sa_attr, 0))
        return false;
    if (!SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0))
        return false;

    HANDLE stderr_read = nullptr, stderr_write = nullptr;
    if (!CreatePipe(&stderr_read, &stderr_write, &sa_attr, 0))
        return false;
    if (!SetHandleInformation(stderr_read, HANDLE_FLAG_INHERIT, 0))
        return false;

    std::string cmdline = "\"" + path + "\"";
    for (auto& arg : args)
        cmdline += " " + arg;

    std::string envstr;
    if (!env.empty()) {
        for (auto& [name, value] : env)
            envstr += name + "=" + value + '\0';
        envstr.push_back('\0');
    }

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = stdin_read;
    si.hStdOutput = stdout_write;
    si.hStdError = stderr_write;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(
            NULL, &cmdline[0], NULL, NULL, TRUE, CREATE_NEW_PROCESS_GROUP,
            envstr.empty() ? NULL : (LPVOID)envstr.c_str(), NULL, &si, &pi)) {
        return false;
    }

    m_pid = pi.dwProcessId;
    m_handle = pi.hProcess;
    m_stdin = stdin_write;
    m_stdout = stdout_read;
    m_stderr = stderr_read;

    CloseHandle(stdin_read);
    CloseHandle(stdout_write);
    CloseHandle(stderr_write);
    CloseHandle(pi.hThread);

    return true;
}

bool subprocess::terminate() {
    if (!m_handle)
        return false;

    if (!TerminateProcess((HANDLE)m_handle, 0))
        return false;

    CloseHandle((HANDLE)m_handle);

    m_pid = 0;
    m_handle = nullptr;
    m_stdin = nullptr;
    m_stdout = nullptr;
    m_stderr = nullptr;

    return true;
}

bool subprocess::write(const string& s) {
    if (!m_stdin)
        return false;

    if (s.empty())
        return true;

    DWORD written = 0;
    if (!WriteFile(m_stdin, s.data(), (DWORD)s.size(), &written, nullptr))
        return false;

    return written == s.size();
}

bool subprocess::read(char* buf, size_t buflen, bool use_stderr) {
    HANDLE fd = use_stderr ? m_stderr : m_stdout;
    if (!fd)
        return false;

    DWORD nread = 0;
    DWORD total = (DWORD)buflen;
    while (nread < total) {
        DWORD count;
        if (!ReadFile(fd, buf + nread, total - nread, &count, NULL))
            return false;
        if (count == 0)
            return false; // EOF

        char* ptr = buf + nread;
        char* src = buf + nread;
        while (src < buf + nread + count) {
            if (*src != '\r')
                *ptr++ = *src++;
            else
                src++;
        }

        nread += (DWORD)(ptr - buf) - nread;
    }

    return true;
}

string subprocess::peek(bool use_stderr) {
    HANDLE fd = use_stderr ? m_stderr : m_stdout;
    if (!fd)
        return "";

    DWORD available = 0;
    if (!PeekNamedPipe(fd, nullptr, 0, nullptr, &available, nullptr))
        MWR_ERROR("PeekNamedPipe: 0x%lx", GetLastError());

    if (available == 0)
        return "";

    DWORD nread;
    vector<char> buf(available + 1);
    if (!ReadFile(fd, buf.data(), available, &nread, nullptr))
        return "";

    // normalize newlines
    string normal;
    for (auto ch : buf) {
        if (ch && ch != '\r')
            normal.push_back(ch);
    }

    return normal;
}

} // namespace mwr
