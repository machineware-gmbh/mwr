/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/utils/library.h"

#include <filesystem>
#include <dlfcn.h>

namespace mwr {

void* library::lookup(const string& name) const {
    void* sym = dlsym(m_handle, name.c_str());
    MWR_REPORT_ON(!sym, "error loading %s: %s", name.c_str(), dlerror());
    return sym;
}

library::library(): m_path(), m_copy(), m_handle(nullptr), m_keep(false) {
    // nothing to do
}

library::library(library&& other) noexcept:
    m_path(std::move(other.m_path)),
    m_copy(std::move(other.m_copy)),
    m_handle(other.m_handle),
    m_keep(other.m_keep) {
    other.m_handle = nullptr;
}

library::library(const string& file, int mode): library() {
    open(file, mode);
}

library::~library() {
    if (!m_keep)
        close();
}

void library::open(const string& path, int mode) {
    if (is_open())
        close();

    if (mode < 0)
        mode = RTLD_NOW | RTLD_LOCAL;

    m_handle = dlopen(path.c_str(), mode);
    MWR_REPORT_ON(!m_handle, "failed to open %s: %s", path.c_str(), dlerror());
    m_path = path;
}

void library::mopen(const string& path, int mode) {
    if (is_open())
        close();

    static unordered_map<string, size_t> maps;
    size_t& count = maps[path];
    if (count > 0) {
        m_copy = mkstr("%s-%zu", path.c_str(), count++);
        if (!std::filesystem::exists(m_copy))
            std::filesystem::copy(path, m_copy);
        open(m_copy, mode);
        m_path = path;
    } else {
        open(path, mode);
        count++;
    }
}

void library::close() {
    if (is_open()) {
        dlclose(m_handle);
        m_handle = nullptr;
    }

    if (!m_copy.empty() && std::filesystem::exists(m_copy)) {
        std::filesystem::remove(m_copy);
        m_copy.clear();
    }
}

bool library::has(const string& name) const {
    void* sym = dlsym(m_handle, name.c_str());
    dlerror(); // clear errors
    return sym != nullptr;
}

} // namespace mwr
