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

#ifdef MWR_WINDOWS
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace mwr {

#ifdef MWR_WINDOWS
static const char* get_last_error() {
    static char buffer[256];
    memset(buffer, 0, sizeof(buffer));

    DWORD err = GetLastError();
    if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, buffer,
                       sizeof(buffer), NULL)) {
        return buffer;
    } 

    snprintf(buffer, sizeof(buffer), "error 0x%08x", err);
    return buffer;
}
#endif

void* library::lookup(const string& name) const {
#ifdef MWR_WINDOWS
    void* sym = GetProcAddress((HMODULE)m_handle, name.c_str());
    MWR_REPORT_ON(!sym, "error loading %s: %s", name.c_str(), get_last_error());
    return sym;
#else
    void* sym = dlsym(m_handle, name.c_str());
    MWR_REPORT_ON(!sym, "error loading %s: %s", name.c_str(), dlerror());
    return sym;
#endif
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

#ifdef MWR_WINDOWS
    m_handle = LoadLibraryA(path.c_str());
    MWR_REPORT_ON(!m_handle, "failed to open dll at %s: %s", path.c_str(),
                  get_last_error());
#else
    if (mode < 0)
        mode = RTLD_NOW | RTLD_LOCAL;

    m_handle = dlopen(path.c_str(), mode);
    MWR_REPORT_ON(!m_handle, "failed to open %s: %s", path.c_str(), dlerror());
#endif
    m_path = path;
}

void library::mopen(const string& path, int mode) {
    static size_t count = 0;
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
#ifdef MWR_WINDOWS
        FreeLibrary((HMODULE)m_handle);
#else
        dlclose(m_handle);
#endif
        m_handle = nullptr;
    }

    if (!m_copy.empty() && std::filesystem::exists(m_copy))
        std::filesystem::remove(m_copy);
}

bool library::has(const string& name) const {
#ifdef MWR_MSVC
    void* sym = GetProcAddress((HMODULE)m_handle, name.c_str());
#else
    void* sym = dlsym(m_handle, name.c_str());
    dlerror(); // clear errors
#endif
    return sym != nullptr;
}

} // namespace mwr
