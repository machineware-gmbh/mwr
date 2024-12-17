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

#ifdef MWR_LINUX
#include <pthread.h>
#include <sched.h>
#endif

#ifdef MWR_MACOS
#include <pthread.h>
#include <mach/mach.h>
#endif

#ifdef MWR_WINDOWS
#include <Windows.h>
#endif

namespace mwr {

static auto current_thread() {
#if defined(MWR_LINUX) || defined(MWR_MACOS)
    return pthread_self();
#elif defined(MWR_WINDOWS)
    return GetCurrentThread();
#endif
}

template <typename THREAD>
static string native_get_thread_name(THREAD handle) {
#if defined(MWR_LINUX) || defined(MWR_MACOS)
    char buffer[256] = {};
    if (pthread_getname_np(handle, buffer, sizeof(buffer)) == 0)
        return buffer;
    return "unknown";
#elif defined(MWR_WINDOWS)
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
#endif
}

template <typename THREAD>
static bool native_set_thread_name(THREAD handle, const string& nm) {
#if defined(MWR_LINUX)
    MWR_ERROR_ON(nm.length() > 15, "thread name too long: %s", nm.c_str());
    return pthread_setname_np(handle, nm.c_str()) == 0;
#elif defined(MWR_MACOS)
    if (handle != pthread_self())
        return false;
    return pthread_setname_np(nm.c_str()) == 0;
#elif defined(MWR_WINDOWS)
    int len = MultiByteToWideChar(CP_UTF8, 0, nm.c_str(), -1, NULL, 0);
    if (len <= 0)
        return false;

    PWSTR wnm = new WCHAR[len];
    MultiByteToWideChar(CP_UTF8, 0, nm.c_str(), -1, wnm, len);
    auto hr = SetThreadDescription(handle, wnm);
    delete[] wnm;
    return SUCCEEDED(hr);
#endif
}

string get_thread_name() {
    return native_get_thread_name(current_thread());
}

string get_thread_name(const thread& t) {
    if (!t.joinable())
        return "unknown";
    auto handle = const_cast<thread&>(t).native_handle();
    return native_get_thread_name(handle);
}

bool set_thread_name(const string& name) {
    return native_set_thread_name(current_thread(), name);
}

bool set_thread_name(thread& t, const string& name) {
    if (!t.joinable())
        return false;

    auto handle = const_cast<thread&>(t).native_handle();
    return native_set_thread_name(handle, name);
}

void set_thread_affinity(int core) {
#if defined(MWR_LINUX)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    if (core >= 0)
        CPU_SET(core, &cpuset);
    else
        CPU_XOR(&cpuset, &cpuset, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
#elif defined(MWR_MACOS)
    thread_affinity_policy_data_t pol;
    pol.affinity_tag = core < 0 ? 0 : core + 1;
    thread_port_t t = pthread_mach_thread_np(pthread_self());
    thread_policy_set(t, THREAD_AFFINITY_POLICY, (thread_policy_t)&pol, 1);
#elif defined(MWR_WINDOWS)
    HANDLE thread = GetCurrentThread();
    DWORD_PTR mask = core < 0 ? ~0ull : 1ull << core;
    SetThreadAffinityMask(thread, mask);
#endif
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
