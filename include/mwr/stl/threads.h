/******************************************************************************
 *                                                                            *
 * Copyright 2022 MachineWare GmbH                                            *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *     http://www.apache.org/licenses/LICENSE-2.0                             *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
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
using std::condition_variable;
using std::condition_variable_any;
using std::lock_guard;

string get_thread_name(const thread& t = std::thread());
bool set_thread_name(thread& t, const string& name);

} // namespace mwr

#endif
