/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_LIBRARY_H
#define MWR_UTILS_LIBRARY_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"
#include "mwr/core/compiler.h"
#include "mwr/core/utils.h"

#include "mwr/stl/strings.h"

namespace mwr {

class library
{
private:
    string m_name;
    string m_path;
    string m_copy;
    void* m_handle;
    bool m_keep;

    void* lookup(const string& name) const;

public:
    const char* name() const { return m_name.c_str(); }
    const char* path() const { return m_path.c_str(); }
    bool is_open() const { return m_handle != nullptr; }

    bool is_kept_alive() const { return m_keep; }
    void keep_alive(bool keep = true) { m_keep = keep; }

    library();
    library(library&& other) noexcept;
    library(const string& path, int mode);
    library(const library& copy) = delete;
    virtual ~library();

    library(const string& path): library(path, -1) {}

    void open(const string& path, int mode = -1);
    void mopen(const string& path, int mode = -1);
    void close();

    bool has(const string& name) const;

    template <typename T>
    void get(T*& fn, const string& name) const;
};

template <typename T>
void library::get(T*& fn, const string& name) const {
    fn = (T*)lookup(name);
}

} // namespace mwr

#endif
