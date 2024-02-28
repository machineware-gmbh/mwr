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

memory::memory(size_t size): m_data(nullptr), m_size(size), m_total_size() {
    m_total_size = (size + page_size() - 1) & ~(page_size() - 1);
    DWORD type = MEM_COMMIT | MEM_RESERVE;
    m_data = (u8*)VirtualAlloc(NULL, size, type, PAGE_READWRITE);
    MWR_ERROR_ON(m_data == NULL, "memory allocation failed");
}

memory::memory(memory&& other) noexcept:
    m_data(other.m_data),
    m_size(other.m_size),
    m_total_size(other.m_total_size) {
    other.m_data = nullptr;
}

memory::~memory() {
    if (m_data != nullptr)
        VirtualFree(m_data, 0, MEM_RELEASE);
}

size_t memory::page_size() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}

} // namespace mwr
