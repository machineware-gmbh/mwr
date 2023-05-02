/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/utils/license.h"

namespace mwr {

const license UNLICENSED("UNLICENSED", "No License/unlicensed", false);

const license KNOWN_LICENSES[] = {
    { "Apache-1.1", "Apache License 1.1" },
    { "Apache-2.0", "Apache License 2.0" },

    { "BSD-1-Clause", "BSD 1-Clause License", false },
    { "BSD-2-Clause", "BSD 2-Clause \"Simplified\" License" },
    { "BSD-3-Clause", "BSD 3-Clause \"New\" or \"Revisited\" License" },

    { "GPL-1.0-only", "GNU General Public License v1.0 only" },
    { "GPL-1.0-or-later", "GNU General Public License v1.0 or later" },
    { "GPL-2.0-only", "GNU General Public License v2.0 only" },
    { "GPL-2.0-or-later", "GNU General Public License v2.0 or later" },
    { "GPL-3.0-only", "GNU General Public License v3.0 only" },
    { "GPL-3.0-or-later", "GNU General Public License v3.0 or later" },

    { "LGPL-2.1-only", "GNU Lesser General Public License v2.1 only" },
    { "LGPL-2.1-or-later", "GNU Lesser General Public License v2.1 or later" },
    { "LGPL-3.0-only", "GNU Lesser General Public License v3.0 only" },
    { "LGPL-3.0-or-later", "GNU Lesser General Public License v3.0 or later" },
};

static unordered_map<string, license*>& all_licenses() {
    static unordered_map<string, license*> all;
    return all;
}

license::license(const string& lic_spdx, const string& nm, bool free):
    spdx(lic_spdx), name(nm), libre(free) {
    all_licenses().insert({ spdx, this });
}

license::~license() {
    all_licenses().erase(spdx);
}

const license& license::find(const string& spdx) {
    auto it = all_licenses().find(spdx);
    return it != all_licenses().end() ? *it->second : UNLICENSED;
}

string license::describe(const string& spdx) {
    const license& lic = find(spdx);
    return lic == UNLICENSED ? spdx : lic.name;
}

} // namespace mwr
