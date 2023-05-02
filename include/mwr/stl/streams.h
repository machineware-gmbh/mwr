/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_STL_STREAMS_H
#define MWR_STL_STREAMS_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

namespace mwr {

using std::istream;
using std::ostream;
using std::fstream;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::ostringstream;
using std::istringstream;

class stream_guard
{
private:
    std::ostream& m_os;
    std::ios::fmtflags m_flags;
    stream_guard() = delete;

public:
    stream_guard(ostream& os): m_os(os), m_flags(os.flags()) {}
    ~stream_guard() { m_os.flags(m_flags); }
};

} // namespace mwr

#endif
