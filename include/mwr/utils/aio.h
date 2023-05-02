/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_AIO_H
#define MWR_UTILS_AIO_H

#include <functional>

namespace mwr {

using std::function;

typedef function<void(int)> aio_handler;

void aio_notify(int fd, aio_handler handler);
void aio_cancel(int fd);

} // namespace mwr

#endif
