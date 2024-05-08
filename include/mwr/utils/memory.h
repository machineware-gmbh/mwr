/******************************************************************************
 *                                                                            *
 * Copyright (C) 2024 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_MEMORY_H
#define MWR_UTILS_MEMORY_H

#include "mwr/core/compiler.h"
#include "mwr/core/types.h"
#include "mwr/core/report.h"

namespace mwr {

class memory
{
private:
    u8* m_data;

    size_t m_size;
    size_t m_total_size;

public:
    constexpr u8* raw() const { return m_data; }
    constexpr size_t size() const { return m_size; }
    constexpr size_t total_size() const { return m_total_size; }

    const u8& operator[](size_t idx) const;
    u8& operator[](size_t idx);
    operator u8*() const { return m_data; }

    memory();
    memory(size_t size);
    memory(memory&& other) noexcept;
    virtual ~memory();

    void alloc(size_t size);
    void alloc(size_t size, int fill);
    void alloc0(size_t size);
    void free();

    memory(const memory&) = delete;

    static size_t page_size();
};

inline const u8& memory::operator[](size_t idx) const {
    MWR_ERROR_ON(idx >= m_size, "memory access out of bounds");
    return m_data[idx];
}

inline u8& memory::operator[](size_t idx) {
    MWR_ERROR_ON(idx >= m_size, "memory access out of bounds");
    return m_data[idx];
}

inline void memory::alloc(size_t size, int fill) {
    alloc(size);
    memset(m_data, fill, m_size);
}

inline void memory::alloc0(size_t size) {
    alloc(size, 0);
}

} // namespace mwr

#endif
