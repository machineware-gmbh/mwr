/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/core/report.h"
#include "mwr/core/utils.h"

namespace mwr {

report::report(const string& msg, const char* file, size_t line):
    std::exception(),
    m_message(msg),
    m_file(file),
    m_line(line),
    m_backtrace(mwr::backtrace(max_backtrace_length, 2)) {
    // nothing to do
}

report::~report() throw() {
    // nothing to do
}

const char* report::what() const throw() {
    return m_message.c_str();
}

void print_error_backtrace(std::ostream& os) {
    auto bt = backtrace(max_backtrace_length, 2);
    if (bt.empty())
        os << "<backtrace unavailable>" << std::endl;
    for (auto it = bt.rbegin(); it != bt.rend(); it++)
        os << *it << std::endl;
}

} // namespace mwr
