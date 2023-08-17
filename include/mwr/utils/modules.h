/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
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
    string license;

    module(string name, string license, size_t version, size_t version_major,
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
    static void print_versions(ostream& os);
    static void print_licenses(ostream& os);
};

#define MWR_DECLARE_MODULE_EX(prefix, name, license, version, major, minor, \
                              patch, verstr, gitrev, shortrev)              \
    MWR_DECL_WEAK extern const mwr::module MWR_CAT(                         \
        module_##prefix##_, version)(name, license, version, major, minor,  \
                                     patch, verstr, gitrev, shortrev);

#define MWR_DECLARE_MODULE(prefix, name, spdx)                             \
    MWR_DECLARE_MODULE_EX(                                                 \
        prefix, name, spdx, MWR_CAT(prefix, _VERSION),                     \
        MWR_CAT(prefix, _VERSION_MAJOR), MWR_CAT(prefix, _VERSION_MINOR),  \
        MWR_CAT(prefix, _VERSION_PATCH), MWR_CAT(prefix, _VERSION_STRING), \
        MWR_CAT(prefix, _GIT_REV), MWR_CAT(prefix, _GIT_REV_SHORT))

#define MWR_DECLARE_MODULE_COPYRIGHT(prefix, name, year) \
    MWR_DECLARE_MODULE(prefix, name,                     \
                       "Copyright " #year                \
                       " MachineWare GmbH. All Rights Reserved")

} // namespace mwr

#endif
