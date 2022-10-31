/******************************************************************************
 *                                                                            *
 * Copyright 2022 MachineWare GmbH                                            *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *     http://www.apache.org/licenses/LICENSE-2.0                             *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
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
