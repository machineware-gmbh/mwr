/******************************************************************************
 *                                                                            *
 * Copyright (C) 2024 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_IHEX_H
#define MWR_UTILS_IHEX_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"

#include "mwr/stl/strings.h"
#include "mwr/stl/containers.h"

namespace mwr {

class ihex
{
public:
    struct record {
        u64 addr;
        vector<u8> data;
    };

    u64 start_addr() const { return m_start_addr; }
    const vector<record>& records() const { return m_records; }

    ihex(const string& filename);
    ~ihex() = default;

    ihex(const ihex&) = delete;

private:
    u64 m_start_addr;
    vector<record> m_records;
};

} // namespace mwr

#endif
