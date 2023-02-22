/******************************************************************************
 *                                                                            *
 * Copyright 2023 MachineWare GmbH                                            *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is unpublished proprietary work and may not be used or disclosed to   *
 * third parties, copied or duplicated in any form, in whole or in part,      *
 * without prior written permission of the authors.                           *
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

} // namespace mwr

#endif
