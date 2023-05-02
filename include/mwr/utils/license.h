/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
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
