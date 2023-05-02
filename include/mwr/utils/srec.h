/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_SREC_H
#define MWR_UTILS_SREC_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"

#include "mwr/stl/strings.h"
#include "mwr/stl/containers.h"

namespace mwr {

class srec
{
public:
    struct record {
        u64 addr;
        vector<u8> data;
    };

    u64 entry() const { return m_entry; }
    string header() const { return m_header; }
    const vector<record>& records() const { return m_records; }

    srec(const string& filename);
    ~srec() = default;

    srec(const srec&) = delete;

private:
    u64 m_entry;
    string m_header;
    vector<record> m_records;
};

} // namespace mwr

#endif
