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

namespace mwr {

subprocess::subprocess():
    m_handle(NULL), m_stdin(NULL), m_stdout(NULL), m_stderr(NULL), env() {
}

subprocess::~subprocess() {
    terminate();
}

bool subprocess::run(const string& path, const vector<string>& args) {
    return false;
}

bool subprocess::terminate() {
    return false;
}

bool subprocess::interrupt() {
    return false;
}

bool subprocess::write(const string& s) {
    return false;
}

string subprocess::read(bool stdout) {
    return string();
}

} // namespace mwr
