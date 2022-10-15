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

MWR_DECLARE_MODULE(MWR, "mwr")

bool module::operator==(const module& other) const {
    return name == other.name && version == other.version &&
           git_rev == other.git_rev;
}

ostream& operator<<(ostream& os, const module& m) {
    os << m.name << ": " << m.version_string;
    return os;
}

void modules::register_module(string name, size_t version,
                              size_t version_major, size_t version_minor,
                              size_t version_patch, string version_string,
                              string git_rev, string git_rev_short) {
    module mod;
    mod.name = std::move(name);
    mod.version_string = std::move(version_string);
    mod.git_rev = std::move(git_rev);
    mod.git_rev_short = std::move(git_rev_short);
    mod.version = version;
    mod.version_major = version_major;
    mod.version_minor = version_minor;
    mod.version_patch = version_patch;
    stl_add_unique(m_modules, mod);
}

modules& modules::instance() {
    static modules singleton;
    return singleton;
}

size_t modules::count() {
    return instance().m_modules.size();
}

const vector<module>& modules::all() {
    return instance().m_modules;
}

const module* modules::find(const string& name) {
    for (const module& mod : all())
        if (mod.name == name)
            return &mod;
    return nullptr;
}

void modules::print_versions(ostream& os) {
    size_t limit = 0;
    for (const module& mod : all())
        limit = max(limit, mod.name.length());

    for (const module& mod : all())
        os << pad(mod.name, limit) << " : " << mod.version_string << std::endl;
}

} // namespace mwr
