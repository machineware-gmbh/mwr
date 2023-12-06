/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
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
using std::map;
using std::multimap;
using std::unordered_set;
using std::unordered_map;
using std::pair;

template <typename T>
inline void stl_remove(vector<T>& v, const T& t) {
    v.erase(std::remove(v.begin(), v.end(), t), v.end());
}

template <typename M, typename T = typename M::mapped_type>
inline void stl_remove(M& m, const T& t) {
    for (auto it = std::begin(m); it != std::end(m);)
        it = it->second == t ? m.erase(it) : ++it;
}

template <typename T, class PRED>
inline void stl_remove_if(vector<T>& v, PRED p) {
    v.erase(std::remove_if(v.begin(), v.end(), p), v.end());
}

template <typename M, class PRED>
inline void stl_remove_if(M& m, PRED p) {
    for (auto it = std::begin(m); it != std::end(m);)
        it = p(it) ? m.erase(it) : ++it;
}

template <typename V, typename T>
inline bool stl_contains(const V& v, const T& t) {
    return std::find(v.begin(), v.end(), t) != v.end();
}

template <typename K, typename V>
inline bool stl_contains(const map<K, V>& m, const K& k) {
    return m.find(k) != m.end();
}

template <typename K, typename V>
inline bool stl_contains(const unordered_map<K, V>& m, const K& k) {
    return m.find(k) != m.end();
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

template <typename V, typename T>
inline void stl_insert_sorted(V& v, const T& t) {
    v.insert(std::upper_bound(v.begin(), v.end(), t), t);
}

template <typename V, typename T, typename PRED>
inline void stl_insert_sorted(V& v, const T& t, PRED p) {
    v.insert(std::upper_bound(v.begin(), v.end(), t, p), t);
}

} // namespace mwr

#endif
