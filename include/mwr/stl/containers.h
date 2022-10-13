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

#ifndef MWR_STL_CONTAINERS_H
#define MWR_STL_CONTAINERS_H

#include <algorithm>
#include <array>
#include <vector>
#include <queue>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace mwr {

using std::list;
using std::queue;
using std::deque;
using std::priority_queue;
using std::array;
using std::vector;
using std::set;
using std::unordered_set;
using std::unordered_map;
using std::pair;

template <typename V, typename T>
inline void stl_remove(V& v, const T& t) {
    v.erase(std::remove(v.begin(), v.end(), t), v.end());
}

template <typename V, class PRED>
inline void stl_remove_if(V& v, PRED p) {
    v.erase(std::remove_if(v.begin(), v.end(), p), v.end());
}

template <typename T1, typename T2, class PRED>
inline void stl_remove_if(std::map<T1, T2>& m, PRED p) {
    for (auto it = std::begin(m); it != std::end(m);)
        it = p(it) ? m.erase(it) : ++it;
}

template <typename T1, typename T2, class PRED>
inline void stl_remove_if(unordered_map<T1, T2>& m, PRED p) {
    for (auto it = std::begin(m); it != std::end(m);)
        it = p(it) ? m.erase(it) : ++it;
}

template <typename V, typename T>
inline bool stl_contains(const V& v, const T& t) {
    return std::find(v.begin(), v.end(), t) != v.end();
}

template <typename T1, typename T2>
inline bool stl_contains(const std::map<T1, T2>& m, const T1& t) {
    return m.find(t) != m.end();
}

template <typename T1, typename T2>
inline bool stl_contains(const std::unordered_map<T1, T2>& m, const T1& t) {
    return m.find(t) != m.end();
}

template <typename V, class PRED>
inline bool stl_contains_if(const V& v, PRED p) {
    return std::find_if(v.begin(), v.end(), p) != v.end();
}

template <typename T>
inline void stl_add_unique(vector<T>& v, const T& t) {
    if (!stl_contains(v, t))
        v.push_back(t);
}

} // namespace mwr

#endif
