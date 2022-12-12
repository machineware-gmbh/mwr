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
#include "mwr/utils/license.h"

namespace mwr {

// clang-format-15 seems to get confused when your class is named 'module', so
// we need to disable formatting here. If we rename this to module1::module1,
// no errors are reported. If we rename it back, the errors return...
// clang-format off
module::module(string nm, string lic, size_t ver, size_t major, size_t minor,
               size_t patch, string verstr, string gitrev,
               string gitrev_short):
    name(std::move(nm)),
    version(ver),
    version_major(major),
    version_minor(minor),
    version_patch(patch),
    version_string(std::move(verstr)),
    git_rev(std::move(gitrev)),
    git_rev_short(std::move(gitrev_short)),
    license(std::move(lic)) {
    MWR_ERROR_ON(name.empty(), "module does not have a name");
    MWR_ERROR_ON(!version, "module does not have a version");
    const module* other = modules::find(name);
    MWR_ERROR_ON(other, "duplicate module found %s: %s vs %s", name.c_str(),
                 version_string.c_str(), other->version_string.c_str());
    modules::instance().import(this);
}
// clang-format on

ostream& operator<<(ostream& os, const module& m) {
    os << m.name << ": " << m.version_string;
    return os;
}

void modules::import(const module* mod) {
    stl_insert_sorted(m_modules, mod, [](const module* a, const module* b) {
        return a->version_string < b->version_string;
    });
}

modules& modules::instance() {
    static modules singleton;
    return singleton;
}

size_t modules::count() {
    return all().size();
}

const vector<const module*>& modules::all() {
    return instance().m_modules;
}

const module* modules::find(const string& name) {
    for (const module* mod : all())
        if (mod->name == name)
            return mod;
    return nullptr;
}

void modules::print_versions(ostream& os) {
    size_t w = 0;
    for (const module* mod : all())
        w = max(w, mod->name.length());

    for (const module* mod : all())
        os << pad(mod->name, w + 1) << mod->version_string << std::endl;
}

void modules::print_licenses(ostream& os) {
    size_t w = 0;
    for (const module* mod : all())
        w = max(w, mod->name.length());

    for (const module* mod : all()) {
        os << pad(mod->name, w + 1) << mod->license << std::endl;
    }
}

} // namespace mwr
