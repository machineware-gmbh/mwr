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

#ifdef MWR_LINUX
#include <link.h>
#endif

#ifdef MWR_MACOS
#include <mach-o/dyld.h>
#endif

namespace mwr {

static vector<string> cleanup_libs;

static void exit_cleanup() {
    for (string& lib : cleanup_libs)
        std::filesystem::remove(lib);
}

static string copy_library(const string& path, size_t n) {
    string name = filename_noext(path);
    string copy = temp_dir() + "/" + name + "-" + to_string(n) + ".so";

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

static string library_path(void* handle, const string& name) {
#ifdef MWR_LINUX
    struct link_map* map;
    if (dlinfo(handle, RTLD_DI_LINKMAP, &map) < 0)
        MWR_ERROR("error dlinfo: %s", dlerror());
    return map->l_name;
#endif
#ifdef MWR_MACOS
    for (u32 i = 0; i < _dyld_image_count(); i++) {
        string path = _dyld_get_image_name(i);
        if (ends_with(path, name))
            return path;
    }

    MWR_ERROR("cannot find path to library %s", name.c_str());
#endif
}

void* library::lookup(const string& name) const {
    void* sym = dlsym(m_handle, name.c_str());
    MWR_REPORT_ON(!sym, "error loading %s: %s", name.c_str(), dlerror());
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

    if (mode < 0)
        mode = RTLD_NOW | RTLD_LOCAL;

    m_handle = dlopen(path.c_str(), mode);
    MWR_REPORT_ON(!m_handle, "failed to open %s: %s", path.c_str(), dlerror());

    if (is_absolute_path(path)) {
        m_path = path;
        m_name = filename(path);
    } else {
        m_name = path;
        m_path = library_path(m_handle, path);
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
        dlclose(m_handle);
        m_handle = nullptr;
    }

    if (!m_copy.empty() && std::filesystem::exists(m_copy)) {
        remove_library(m_copy);
        m_copy.clear();
    }

    m_name.clear();
    m_path.clear();
}

bool library::has(const string& name) const {
    void* sym = dlsym(m_handle, name.c_str());
    dlerror(); // clear errors
    return sym != nullptr;
}

} // namespace mwr
