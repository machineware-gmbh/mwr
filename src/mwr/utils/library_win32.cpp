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
#include <Windows.h>

namespace mwr {

static vector<string> cleanup_libs;

static void exit_cleanup() {
    for (string& lib : cleanup_libs)
        std::filesystem::remove(lib);
}

static string copy_library(const string& path, size_t n) {
    string name = filename_noext(path);
    string copy = temp_dir() + "/" + name + "-" + to_string(n) + ".dll";

    auto options = std::filesystem::copy_options::update_existing |
                   std::filesystem::copy_options::create_hard_links;
    std::filesystem::copy_file(path, copy, options);

    if (cleanup_libs.empty())
        std::atexit(exit_cleanup);
    stl_add_unique(cleanup_libs, copy);

    return copy;
}

static void remove_library(const string& path) {
    std::filesystem::remove(path);
    stl_remove(cleanup_libs, path);
}

static const char* library_strerror(DWORD err = GetLastError()) {
    static char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, buffer,
                       sizeof(buffer), NULL)) {
        return buffer;
    }

    snprintf(buffer, sizeof(buffer), "error 0x%08x", err);
    return buffer;
}

static string library_path(void* handle) {
    char path[MAX_PATH];
    DWORD result = GetModuleFileNameA((HMODULE)handle, path, MAX_PATH);
    MWR_ERROR_ON(!result, "GetModuleFileName: %s", library_strerror());
    return path;
}

void* library::lookup(const string& name) const {
    void* sym = GetProcAddress((HMODULE)m_handle, name.c_str());
    MWR_REPORT_ON(!sym, "error loading %s: %s", name.c_str(),
                  library_strerror());
    return sym;
}

library::library():
    m_name(), m_path(), m_copy(), m_handle(nullptr), m_keep(false) {
    // nothing to do
}

library::library(library&& other) noexcept:
    m_name(std::move(other.m_name)),
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

    m_handle = LoadLibraryA(path.c_str());
    MWR_REPORT_ON(!m_handle, "failed to open dll at %s: %s", path.c_str(),
                  library_strerror());

    if (is_absolute_path(path)) {
        m_path = path;
        m_name = filename(path);
    } else {
        m_name = path;
        m_path = library_path(m_handle);
    }
}

void library::mopen(const string& path, int mode) {
    if (is_open())
        close();

    string name = path;
    if (is_absolute_path(path))
        name = filename(name);

    struct openinfo {
        string path;
        size_t copies;
    };

    static unordered_map<string, openinfo> maps;
    openinfo& info = maps[name];

    if (info.copies == 0) {
        open(path, mode);
        info.path = m_path;
        info.copies++;
    } else {
        m_copy = copy_library(info.path, info.copies++);
        open(m_copy, mode);
        m_name = name;
    }
}

void library::close() {
    if (is_open()) {
        FreeLibrary((HMODULE)m_handle);
        m_handle = nullptr;
    }

    if (!m_copy.empty() && std::filesystem::exists(m_copy)) {
        remove_library(m_copy);
        m_copy.clear();
    }
}

bool library::has(const string& name) const {
    void* sym = GetProcAddress((HMODULE)m_handle, name.c_str());
    return sym != nullptr;
}

} // namespace mwr
