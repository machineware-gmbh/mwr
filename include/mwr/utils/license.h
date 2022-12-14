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

#ifndef MWR_UTILS_LICENSE_H
#define MWR_UTILS_LICENSE_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"
#include "mwr/core/compiler.h"

#include "mwr/stl/strings.h"
#include "mwr/stl/streams.h"
#include "mwr/stl/containers.h"

namespace mwr {

struct license {
    const string spdx;
    const string name;
    const bool libre;

    license() = delete;
    license(license&&) = delete;
    license(const license&) = delete;

    license(const string& lic_spdx, const string& lic_name, bool libre = true);
    ~license();

    bool operator==(const license& o) const { return spdx == o.spdx; }
    bool operator!=(const license& o) const { return spdx != o.spdx; }

    static const license& find(const string& spdx);
    static string describe(const string& spdx);
};

extern const license UNLICENSED;

} // namespace mwr

#endif
