/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_SFINAE_H
#define MWR_SFINAE_H

#include <type_traits>

namespace mwr {

template <typename T>
using enable_if_signed = typename std::enable_if<std::is_signed_v<T>, T>;

template <typename T>
using enable_if_signed_t = typename enable_if_signed<T>::type;

template <typename T>
using enable_if_unsigned = typename std::enable_if<std::is_unsigned_v<T>, T>;

template <typename T>
using enable_if_unsigned_t = typename enable_if_unsigned<T>::type;

template <typename BASE, typename DERIVED>
using enable_if_base_if = typename std::enable_if<
    std::is_base_of_v<BASE, std::decay_t<DERIVED>>, BASE>;

template <typename B, typename D>
using enable_if_base_of_t = typename enable_if_base_if<B, D>::type;

#define MWR_DECL_MEMBER_CHECKER(T, MEMBER)                     \
    struct member_checker_##T##_has_##MEMBER {                 \
        template <typename T, typename U = void>               \
        struct has_member : std::false_type {};                \
        template <typename T>                                  \
        struct has_member<T, std::void_t<decltype(T::MEMBER)>> \
            : std::true_type {};                               \
        constexpr operator bool() {                            \
            return has_member<T>::value;                       \
        }                                                      \
    }

} // namespace mwr

#endif
