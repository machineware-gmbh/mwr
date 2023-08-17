/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/utils/options.h"
#include "mwr/core/utils.h"

namespace mwr {

bool option_base::matches(const char* argv) const {
    if (!argv || !argv[0])
        return false;
    if (!m_name.empty() && m_name.compare(argv) == 0)
        return true;
    if (!m_aname.empty() && m_aname.compare(argv) == 0)
        return true;
    return false;
}

option_base::option_base(const string& name, const string& aname,
                         const string& desc, size_t argc):
    m_name(name), m_aname(aname), m_desc(desc), m_argc(argc) {
    MWR_ERROR_ON(m_name.empty(), "option name cannot be empty");
    MWR_ERROR_ON(m_name[0] != '-', "option name invalid");
    if (!m_aname.empty())
        MWR_ERROR_ON(m_aname[0] != '-', "alternative option name invalid");
    options::instance().register_option(this);
}

option_base::~option_base() {
    options::instance().unregister_option(this);
}

option<bool>::option(const string& name, const string& desc):
    option(name, "", desc) {
}

option<bool>::option(const string& name, const string& desc,
                     function<bool(void)> parse):
    option(name, "", desc, std::move(parse)) {
}

option<bool>::option(const string& name, const string& aname,
                     const string& desc):
    option(name, aname, desc, [&]() -> bool {
        m_has_value = true;
        m_value = !m_value;
        return true;
    }) {
}

option<bool>::option(const string& name, const string& aname,
                     const string& desc, function<bool(void)> parse):
    option_base(name, aname, desc, 0),
    m_value(false),
    m_has_value(false),
    m_parse(std::move(parse)) {
}

bool option<bool>::parse(int argc, const char* const* argv,
                         const function<bool(void)>& parse) {
    m_value = false;
    m_has_value = false;

    int i = 0;
    while (i < argc) {
        if (!matches(argv[i++]))
            continue;

        if (parse && !parse())
            return true;
        if (m_parse && !m_parse())
            return true;
    }

    return true;
}

bool option<bool>::parse(int argc, const char* const* argv) {
    return parse(argc, argv, function<bool(void)>());
}

void options::register_option(option_base* opt) {
    for (option_base* other : m_options) {
        if (other->matches(opt->name()))
            MWR_ERROR("option %s already defined", opt->name());
        if (other->matches(opt->aname()))
            MWR_ERROR("option %s already defined", opt->aname());
    }

    stl_add_unique(m_options, opt);
}

void options::unregister_option(option_base* opt) {
    stl_remove(m_options, opt);
}

options& options::instance() {
    static options singleton;
    return singleton;
}

const vector<option_base*>& options::all() {
    return instance().m_options;
}

option_base* options::find(const string& name) {
    for (option_base* opt : all())
        if (opt->matches(name.c_str()))
            return opt;
    return nullptr;
}

bool options::parse(int argc, const char* const* argv) {
    bool result = true;
    for (option_base* opt : all())
        result &= opt->parse(argc, argv);
    return result;
}

bool options::parse(int argc, const char* const* argv, vector<string>& extra) {
    if (!parse(argc, argv))
        return false;

    int i = 1;
    while (i < argc) {
        if (argv[i][0] == '-') {
            option_base* opt = find(argv[i]);
            i += 1 + (opt ? (int)opt->argc() : 0);
        } else {
            extra.push_back(argv[i++]);
        }
    }

    return true;
}

void options::print_help(ostream& os) {
    struct help {
        string mnem;
        string desc;

        bool operator<(const help& other) const { return mnem < other.mnem; }
    };

    size_t column = 0;
    vector<help> entries;

    for (option_base* opt : all()) {
        string mnemonic(opt->name());
        if (opt->aname())
            mnemonic += mkstr(", %s", opt->aname());
        if (opt->argc())
            mnemonic += " <value>";

        column = max(column, mnemonic.size());
        entries.push_back({ mnemonic, opt->desc() });
    }

    std::sort(entries.begin(), entries.end());

    for (const auto& entry : entries)
        os << pad(entry.mnem, column) << " " << entry.desc << std::endl;
}

} // namespace mwr
