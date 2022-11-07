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

#ifndef MWR_UTILS_MODULES_H
#define MWR_UTILS_MODULES_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"
#include "mwr/core/compiler.h"

#include "mwr/stl/strings.h"
#include "mwr/stl/streams.h"
#include "mwr/stl/containers.h"

namespace mwr {

struct module {
    string name;
    size_t version;
    size_t version_major;
    size_t version_minor;
    size_t version_patch;
    string version_string;
    string git_rev;
    string git_rev_short;

    module(string name, size_t version, size_t version_major,
           size_t version_minor, size_t version_patch, string version_string,
           string git_rev, string git_rev_short);
};

ostream& operator<<(ostream& os, const module& mod);

class modules
{
    friend struct module;

private:
    vector<const module*> m_modules;

    modules() = default;

    void import(const module* mod);

    static modules& instance();

public:
    ~modules() = default;

    static size_t count();
    static const vector<const module*>& all();
    static const module* find(const string& name);
    static void print_modules(ostream& os);
};

#define MWR_DECLARE_MODULE_EX(prefix, name, version, major, minor, patch, \
                              verstr, gitrev, shortrev)                   \
    MWR_DECL_WEAK mwr::module MWR_CAT(module_##prefix##_, version)(       \
        name, version, major, minor, patch, verstr, gitrev, shortrev);

#define MWR_DECLARE_MODULE(prefix, name)                                   \
    MWR_DECLARE_MODULE_EX(                                                 \
        prefix, name, MWR_CAT(prefix, _VERSION),                           \
        MWR_CAT(prefix, _VERSION_MAJOR), MWR_CAT(prefix, _VERSION_MINOR),  \
        MWR_CAT(prefix, _VERSION_PATCH), MWR_CAT(prefix, _VERSION_STRING), \
        MWR_CAT(prefix, _GIT_REV), MWR_CAT(prefix, _GIT_REV_SHORT))

} // namespace mwr

#endif
