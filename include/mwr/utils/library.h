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

#ifndef MWR_UTILS_LIBRARY_H
#define MWR_UTILS_LIBRARY_H

#include "mwr/core/types.h"
#include "mwr/core/report.h"
#include "mwr/core/compiler.h"

#include "mwr/stl/strings.h"

namespace mwr {

class library
{
private:
    string m_path;
    void* m_handle;
    bool m_keep;

    void* lookup(const string& name) const;

public:
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
