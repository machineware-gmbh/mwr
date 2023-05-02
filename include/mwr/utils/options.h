/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_OPTIONS_H
#define MWR_UTILS_OPTIONS_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"
#include "mwr/core/compiler.h"

#include "mwr/stl/strings.h"
#include "mwr/stl/streams.h"
#include "mwr/stl/containers.h"

namespace mwr {

class option_base
{
private:
    string m_name;
    string m_aname;
    string m_desc;
    size_t m_argc;

public:
    const char* name() const { return m_name.c_str(); }
    const char* desc() const { return m_desc.c_str(); }
    size_t argc() const { return m_argc; }

    const char* aname() const;

    bool matches(const char* name) const;

    option_base(const string& name, const string& aname, const string& desc,
                size_t argc);
    virtual ~option_base();

    virtual bool parse(int argc, const char* const* argv) = 0;
};

inline const char* option_base::aname() const {
    return m_aname.empty() ? nullptr : m_aname.c_str();
}

template <typename T>
class option : public option_base
{
private:
    vector<T> m_values;
    function<bool(T)> m_parse;

public:
    size_t count() const { return m_values.size(); }
    bool has_value() const { return count() > 0; }
    const vector<T>& values() const { return m_values; }
    T values(size_t idx) const { return m_values.at(idx); }
    T value() const { return values(count() - 1); }
    operator T() const { return value(); }

    bool operator==(T other) const { return has_value() && value() == other; }
    bool operator!=(T other) const { return has_value() && value() != other; }
    bool operator<=(T other) const { return has_value() && value() <= other; }
    bool operator>=(T other) const { return has_value() && value() >= other; }
    bool operator<(T other) const { return has_value() && value() < other; }
    bool operator>(T other) const { return has_value() && value() > other; }

    option(const string& name, const string& desc): option(name, "", desc) {}
    option(const string& name, const string& desc, function<bool(T)> parse);
    option(const string& name, const string& aname, const string& desc);
    option(const string& name, const string& aname, const string& desc,
           function<bool(T)> parse);

    virtual bool parse(int argc, const char* const* argv,
                       const function<bool(T)>& parse);
    virtual bool parse(int argc, const char* const* argv) override;
};

template <typename T>
inline option<T>::option(const string& name, const string& desc,
                         function<bool(T)> parse):
    option(name, "", desc, std::move(parse)) {
}

template <typename T>
inline option<T>::option(const string& name, const string& aname,
                         const string& desc):
    option(name, aname, desc, [&](const T& val) -> bool {
        m_values.push_back(val);
        return true;
    }) {
}

template <typename T>
inline option<T>::option(const string& name, const string& aname,
                         const string& desc, function<bool(T)> parse):
    option_base(name, aname, desc, 1), m_values(), m_parse(std::move(parse)) {
}

template <typename T>
inline bool option<T>::parse(int argc, const char* const* argv,
                             const function<bool(T)>& parse) {
    m_values.clear();

    int i = 0;
    while (i < argc) {
        if (!matches(argv[i++]))
            continue;
        if (i >= argc)
            return false;

        string str = argv[i++];
        if (str.empty() || str[0] == '-')
            return false;

        T val = from_string<T>(str);
        if (parse && !parse(val))
            return true;
        if (m_parse && !m_parse(val))
            return true;
    }

    return true;
}

template <typename T>
inline bool option<T>::parse(int argc, const char* const* argv) {
    return parse(argc, argv, function<bool(T)>());
}

template <>
class option<bool> : public option_base
{
private:
    bool m_value;
    bool m_has_value;
    function<bool(void)> m_parse;

public:
    bool has_value() const { return m_has_value; }
    bool value() const { return m_value; }
    operator bool() const { return value(); }

    option(const string& name, const string& desc);
    option(const string& name, const string& desc, function<bool(void)> parse);
    option(const string& name, const string& aname, const string& desc);
    option(const string& name, const string& aname, const string& desc,
           function<bool(void)> parse);

    virtual bool parse(int argc, const char* const* argv,
                       const function<bool(void)>& parse);
    virtual bool parse(int argc, const char* const* argv) override;
};

class options
{
    friend class option_base;

private:
    vector<option_base*> m_options;

    options() = default;

    void register_option(option_base* opt);
    void unregister_option(option_base* opt);

    static options& instance();

public:
    static const vector<option_base*>& all();
    static option_base* find(const string& name);
    static bool parse(int argc, const char* const* argv);
    static bool parse(int argc, const char* const* argv,
                      vector<string>& extra);

    static void print_help(ostream& os);
};

} // namespace mwr

#endif
