/******************************************************************************
 *                                                                            *
 * Copyright (C) 2024 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/utils/memory.h"

#include <unistd.h>
#include <sys/mman.h>

namespace mwr {

memory::memory(size_t size): m_data(nullptr), m_size(size), m_total_size() {
    m_total_size = (size + page_size() - 1) & ~(page_size() - 1);
    int perms = PROT_READ | PROT_WRITE;
    int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
    m_data = (u8*)mmap(NULL, m_total_size, perms, flags, -1, 0);
    MWR_ERROR_ON((void*)m_data == MAP_FAILED, "memory allocation failed");
}

memory::memory(memory&& other) noexcept:
    m_data(other.m_data),
    m_size(other.m_size),
    m_total_size(other.m_total_size) {
    other.m_data = nullptr;
}

memory::~memory() {
    if (m_data != nullptr)
        munmap(m_data, m_total_size);
}

size_t memory::page_size() {
    return sysconf(_SC_PAGE_SIZE);
}

} // namespace mwr
