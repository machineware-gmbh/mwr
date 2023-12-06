/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_STL_THREADS_H
#define MWR_STL_THREADS_H

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace mwr {

using std::string;
using std::atomic;
using std::thread;
using std::mutex;
using std::recursive_mutex;
using std::condition_variable;
using std::condition_variable_any;
using std::lock_guard;

string get_thread_name();
string get_thread_name(const thread& t);

bool set_thread_name(const string& name);
bool set_thread_name(thread& t, const string& name);

void sleep(unsigned long long seconds);
void usleep(unsigned long long microseconds);
void nanosleep(unsigned long long nanoseconds);

} // namespace mwr

#endif
