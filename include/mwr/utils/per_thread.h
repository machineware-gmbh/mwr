/******************************************************************************
 *                                                                            *
 * Copyright (C) 2025 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_PER_THREAD_H
#define MWR_UTILS_PER_THREAD_H

#include <mutex>
#include <thread>
#include <unordered_map>

namespace mwr {

struct per_thread_id {
    typedef std::thread::id id_t;
    id_t operator()() const { return std::this_thread::get_id(); }
};

template <typename T, typename ID = per_thread_id>
class per_thread
{
public:
    typedef typename ID::id_t id_t;
    void set(const T& val, id_t id = ID()()) {
        std::lock_guard<std::recursive_mutex> lock(m_mtx);
        m_storage[id] = val;
    }

    void set(T&& val, id_t id = ID()()) {
        std::lock_guard<std::recursive_mutex> lock(m_mtx);
        m_storage[id] = std::move(val);
    }

    T& get(id_t id = ID()()) {
        std::lock_guard<std::recursive_mutex> lock(m_mtx);
        return m_storage[id];
    }

    const T& get(id_t id = ID()()) const {
        std::lock_guard<std::recursive_mutex> lock(m_mtx);
        return m_storage.at(id);
    }

    void clear(id_t id = ID()()) {
        std::lock_guard<std::recursive_mutex> lock(m_mtx);
        m_storage.erase(id);
    }

    bool has_value(id_t id = ID()()) const {
        std::lock_guard<std::recursive_mutex> lock(m_mtx);
        return m_storage.find(id) != m_storage.end();
    }

    void lock() const { m_mtx.lock(); }
    void unlock() const { m_mtx.unlock(); }
    explicit operator bool() const { return has_value(); }
    operator T() const { return get(); }
    T& operator*() { return get(); }
    const T& operator*() const { return get(); }
    T* operator->() { return &get(); }
    const T* operator->() const { return &get(); }
    void operator=(const T& val) { set(val); }
    void operator=(T&& val) { set(std::move(val)); }
    const T& operator[](id_t id) const { return get(id); }
    T& operator[](id_t id) { return get(id); }

private:
    mutable std::recursive_mutex m_mtx;
    std::unordered_map<id_t, T> m_storage;
};

} // namespace mwr

#endif
