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

namespace mwr {

string get_thread_name(const thread& t) {
#ifdef __linux__
    thread::native_handle_type handle = const_cast<thread&>(t).native_handle();
    if (!t.joinable())
        handle = (thread::native_handle_type)pthread_self();

    char buffer[256] = {};
    if (pthread_getname_np(handle, buffer, sizeof(buffer)) != 0)
        return "unknown";

    return buffer;
#else
    return "unknown";
#endif
}

bool set_thread_name(thread& t, const string& nm) {
#ifdef __linux__
    MWR_ERROR_ON(nm.length() > 15, "thread name too long: %s", nm.c_str());
    return pthread_setname_np(t.native_handle(), nm.c_str()) == 0;
#else
    return false;
#endif
}

} // namespace mwr
