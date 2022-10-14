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

    bool operator==(const module& other) const;
};

ostream& operator<<(ostream& os, const module& mod);

class modules
{
private:
    vector<module> m_modules;

    modules() = default;

public:
    ~modules() = default;

    void register_module(const string& name, size_t version,
                         size_t version_major, size_t version_minor,
                         size_t version_patch, const string& version_string,
                         const string& git_rev, const string& git_rev_short);

    static modules& instance();
    static const vector<module>& all();
    static const module* find(const string& name);
};

#define MWR_DECLARE_MODULE(name, prefix)                                      \
    extern "C" MWR_DECL_CONSTRUCTOR void name##_module_register() {           \
        ::mwr::modules::instance().register_module(                           \
            #name, MWR_CAT(prefix, _VERSION),                                 \
            MWR_CAT(prefix, _VERSION_MAJOR), MWR_CAT(prefix, _VERSION_MINOR), \
            MWR_CAT(prefix, _VERSION_PATCH),                                  \
            MWR_CAT(prefix, _VERSION_STRING), MWR_CAT(prefix, _GIT_REV),      \
            MWR_CAT(prefix, _GIT_REV_SHORT));                                 \
    }

} // namespace mwr

#endif
