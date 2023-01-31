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

#ifndef MWR_COMPILER_H
#define MWR_COMPILER_H

#define MWR_DECL_ALIGN(n)     __attribute__((aligned(n)))
#define MWR_DECL_PACKED       __attribute__((packed))
#define MWR_DECL_PRINTF(s, a) __attribute__((format(printf, s, a)))
#define MWR_DECL_CONSTRUCTOR  __attribute__((constructor))
#define MWR_DECL_DESTRUCTOR   __attribute__((destructor))
#define MWR_DECL_WEAK         __attribute__((weak))
#define MWR_DECL_USED         __attribute__((used))
#define MWR_DECL_UNUSED       __attribute__((unused))
#define MWR_DECL_INLINE       __attribute__((always_inline))
#define MWR_DECL_NOINLINE     __attribute__((noinline))
#define MWR_DECL_NORETURN     __attribute__((noreturn))
#define MWR_DECL_DEPRECATED   __attribute__((deprecated))

#define MWR_UNREACHABLE __builtin_unreachable()

#define MWR_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define MWR_NOP(val)   val
#define MWR_XCAT(a, b) a##b
#define MWR_CAT(a, b)  MWR_XCAT(a, b)
#define MWR_XSTR(str)  #str
#define MWR_STR(str)   MWR_XSTR(str)

namespace mwr {

template <typename T>
constexpr int likely(const T& x) {
    return __builtin_expect(!!(x), 1);
}

template <typename T>
constexpr int unlikely(const T& x) {
    return __builtin_expect(!!(x), 0);
}

} // namespace mwr

#endif
