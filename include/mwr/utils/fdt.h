/******************************************************************************
 *                                                                            *
 * Copyright (C) 2025 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_FDT_H
#define MWR_UTILS_FDT_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"

#include "mwr/stl/strings.h"
#include "mwr/stl/containers.h"

namespace mwr {

class fdtprop
{
private:
    string m_name;
    size_t m_size;
    vector<u32> m_numbers;
    vector<string> m_strings;

public:
    const string& name() const { return m_name; }
    size_t size() const { return m_size; }

    const vector<u32>& numbers() const { return m_numbers; }
    const vector<string>& strings() const { return m_strings; }

    fdtprop(const string& name): m_name(name), m_numbers(), m_strings() {}
    fdtprop(const string& name, vector<u32> numbers);
    fdtprop(const string& name, vector<string> strings);
    ~fdtprop() = default;

    void dump(ostream& os, size_t indent = 0) const;
};

class fdtnode
{
private:
    string m_name;
    vector<fdtprop*> m_props;
    vector<fdtnode*> m_children;

public:
    const string& name() const { return m_name; }
    const vector<fdtprop*>& properties() const { return m_props; }
    const vector<fdtnode*>& children() const { return m_children; }

    fdtnode(const string& name): m_name(name), m_props(), m_children() {}
    ~fdtnode();
    fdtnode(const fdtnode&) = delete;
    fdtnode(fdtnode&& other) = default;

    u32 handle();

    void dump(ostream& os, size_t indent = 0) const;

    fdtprop* find_property(const string& name) {
        for (auto* prop : m_props)
            if (prop->name() == name)
                return prop;
        return nullptr;
    }

    void add_property(fdtprop* prop) { m_props.push_back(prop); }

    void add_property(const string& name) { add_property(new fdtprop(name)); }

    void add_property(const string& name, u32 value) {
        add_property(name, { value });
    }

    void add_property(const string& name, const string& value) {
        add_property(name, { value });
    }

    void add_property(const string& name, std::initializer_list<u32> values) {
        add_property(new fdtprop(name, values));
    }

    void add_property(const string& name, std::initializer_list<string> val) {
        add_property(new fdtprop(name, val));
    }

    fdtnode& add_child(const string& name) {
        m_children.push_back(new fdtnode(name));
        return *m_children.back();
    }

    fdtnode* find_child(const string& name) {
        for (auto* child : m_children)
            if (child->name() == name)
                return child;
        return nullptr;
    }
};

ostream& operator<<(ostream& os, const fdtprop& prop);
ostream& operator<<(ostream& os, const fdtnode& node);

void fdtcompile(const fdtnode& root, void* buffer, size_t buflen);
void fdtcompile(const fdtnode& root, const string& filename);

fdtnode fdtdecompile(const void* buffer, size_t buflen);
fdtnode fdtdecompile(const string& filename);

} // namespace mwr

#endif
