/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/core/report.h"
#include "mwr/stl/threads.h"

#ifdef _MSC_VER
#include <Windows.h>
#endif

namespace mwr {

string get_thread_name(const thread& t) {
#if defined(MWR_LINUX) || defined(MWR_MACOS)
    thread::native_handle_type handle = const_cast<thread&>(t).native_handle();
    if (!t.joinable())
        handle = (thread::native_handle_type)pthread_self();

    char buffer[256] = {};
    if (pthread_getname_np(handle, buffer, sizeof(buffer)) != 0)
        return "unknown";

    return buffer;
#elif defined(MWR_WINDOWS)
    thread::native_handle_type handle = const_cast<thread&>(t).native_handle();
    PWSTR name = nullptr;
    auto hr = GetThreadDescription(handle, &name);
    if (FAILED(hr))
        return "unknown";

    int len = WideCharToMultiByte(CP_UTF8, 0, name, -1, NULL, 0, NULL, NULL);
    if (len <= 0)
        return "unknown";

    string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, name, -1, &result[0], len, NULL, NULL);
    return result;
#else
    return "unknown";
#endif
}

bool set_thread_name(thread& t, const string& nm) {
#if defined(MWR_LINUX)
    MWR_ERROR_ON(nm.length() > 15, "thread name too long: %s", nm.c_str());
    return pthread_setname_np(t.native_handle(), nm.c_str()) == 0;
#elif defined(MWR_MACOS)
    if (t.get_id() != std::this_thread::get_id())
        return false;
    return pthread_setname_np(nm.c_str()) == 0;
#elif defined(MWR_WINDOWS)
    int len = MultiByteToWideChar(CP_UTF8, 0, nm.c_str(), -1, NULL, 0);
    if (len <= 0)
        return false;

    PWSTR wnm = new WCHAR[len];
    MultiByteToWideChar(CP_UTF8, 0, nm.c_str(), -1, wnm, len);
    auto hr = SetThreadDescription(t.native_handle(), wnm);
    delete[] wnm;
    return SUCCEEDED(hr);
#endif
    return false;
}

void sleep(unsigned long long seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void usleep(unsigned long long microseconds) {
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

void nanosleep(unsigned long long nanoseconds) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(nanoseconds));
}

} // namespace mwr
