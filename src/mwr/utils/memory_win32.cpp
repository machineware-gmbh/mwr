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

#include <Windows.h>

namespace mwr {

memory::memory(): m_data(nullptr), m_size(), m_total_size() {
    // nothing to do
}

memory::memory(size_t size): m_data(nullptr), m_size(), m_total_size() {
    alloc(size);
}

memory::memory(memory&& other) noexcept:
    m_data(other.m_data),
    m_size(other.m_size),
    m_total_size(other.m_total_size) {
    other.m_data = nullptr;
}

memory::~memory() {
    free();
}

void memory::alloc(size_t size) {
    MWR_ERROR_ON(m_data, "memory already allocated");
    MWR_ERROR_ON(size == 0, "attempt to allocate zero bytes");

    m_size = size;
    m_total_size = (size + page_size() - 1) & ~(page_size() - 1);

    DWORD type = MEM_COMMIT | MEM_RESERVE;
    m_data = (u8*)VirtualAlloc(NULL, size, type, PAGE_READWRITE);
    MWR_ERROR_ON(m_data == NULL, "memory allocation failed");
}

void memory::free() {
    if (m_data != nullptr) {
        VirtualFree(m_data, 0, MEM_RELEASE);
        m_data = nullptr;
        m_size = 0;
        m_total_size = 0;
    }
}

size_t memory::page_size() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}

} // namespace mwr
