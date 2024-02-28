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

    memory(size_t size);
    memory(memory&& other) noexcept;
    virtual ~memory();

    memory(const memory&) = delete;

    static size_t page_size();
};

} // namespace mwr

#endif
