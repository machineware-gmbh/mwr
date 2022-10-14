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

#ifndef MWR_CORE_COMPILER_H
#define MWR_CORE_COMPILER_H

#include <stdio.h>

#define MWR_DECL_PACKED __attribute__((packed))
#define MWR_DECL_PRINTF(strpos, argpos) \
    __attribute__((format(printf, (strpos), (argpos))))
#define MWR_DECL_CONSTRUCTOR __attribute__((constructor))
#define MWR_DECL_DESTRUCTOR  __attribute__((destructor))

#define MWR_ERROR(...)                                 \
    do {                                               \
        fprintf(stderr, "%s:%d ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                  \
        fprintf(stderr, "\n");                         \
        fflush(stderr);                                \
        abort();                                       \
    } while (0)

#define MWR_ERROR_ON(cond, ...)     \
    do {                            \
        if (mwr::unlikely(cond)) {  \
            MWR_ERROR(__VA_ARGS__); \
        }                           \
    } while (0)

#define MWR_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

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
