/******************************************************************************
 *                                                                            *
 * Copyright (C) 2025 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_SUBPROCESS_H
#define MWR_UTILS_SUBPROCESS_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"
#include "mwr/core/compiler.h"
#include "mwr/core/utils.h"

#include "mwr/stl/strings.h"
#include "mwr/stl/containers.h"

namespace mwr {

class subprocess
{
private:
#ifdef MWR_WINDOWS
    void* m_handle;
    void* m_stdin;
    void* m_stdout;
    void* m_stderr;
    unsigned long m_pid;
#else
    int m_pid;
    int m_stdin;
    int m_stdout;
    int m_stderr;
#endif

public:
    map<string, string> env;

#ifdef MWR_WINDOWS
    unsigned long pid() const { return m_pid; }
    void* native_handle() const { return m_handle; }
#else
    int pid() const { return m_pid; }
    int native_handle() const { return m_pid; }
#endif

    subprocess();
    virtual ~subprocess();

    bool run(const string& path,
             const vector<string>& args = vector<string>());

    bool terminate();

    bool write(const string& str);
    bool read(char* buf, size_t buflen, bool use_stderr = false);

    string peek(bool use_stderr = false); // non-blocking
};

} // namespace mwr

#endif
