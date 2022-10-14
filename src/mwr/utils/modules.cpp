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

#include "mwr/utils/modules.h"

namespace mwr {

bool module::operator==(const module& other) const {
    return name == other.name && version == other.version &&
           git_rev == other.git_rev;
}

ostream& operator<<(ostream& os, const module& m) {
    os << m.name << ": " << m.version_string << " (" << m.git_rev << ")";
    return os;
}

const module* modules::find(const string& name) const {
    for (const module& mod : instance().m_modules)
        if (mod.name == name)
            return &mod;
    return nullptr;
}

void modules::register_module(const string& name, size_t version,
                              size_t version_major, size_t version_minor,
                              size_t version_patch,
                              const string& version_string,
                              const string& git_rev,
                              const string& git_rev_short) {
    module mod;
    mod.name = name;
    mod.version = version;
    mod.version_major = version_major;
    mod.version_minor = version_minor;
    mod.version_patch = version_patch;
    mod.version_string = version_string;
    mod.git_rev = git_rev;
    mod.git_rev_short = git_rev_short;

    replace(mod.name, "_", "-");

    if (!stl_contains(m_modules, mod))
        m_modules.push_back(std::move(mod));
}

modules& modules::instance() {
    static modules inst;
    return inst;
}

} // namespace mwr
