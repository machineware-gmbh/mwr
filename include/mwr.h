/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_H
#define MWR_H

#include "mwr/core/compiler.h"
#include "mwr/core/types.h"
#include "mwr/core/sfinae.h"
#include "mwr/core/report.h"
#include "mwr/core/atomics.h"
#include "mwr/core/bitops.h"
#include "mwr/core/bitfields.h"
#include "mwr/core/muldiv.h"
#include "mwr/core/saturate.h"
#include "mwr/core/utils.h"

#include "mwr/stl/containers.h"
#include "mwr/stl/strings.h"
#include "mwr/stl/streams.h"
#include "mwr/stl/threads.h"

#include "mwr/logging/publisher.h"
#include "mwr/logging/publishers/file.h"
#include "mwr/logging/publishers/stream.h"
#include "mwr/logging/publishers/terminal.h"
#include "mwr/logging/logger.h"

#include "mwr/utils/aio.h"
#include "mwr/utils/elf.h"
#include "mwr/utils/license.h"
#include "mwr/utils/library.h"
#include "mwr/utils/locale.h"
#include "mwr/utils/memory.h"
#include "mwr/utils/modules.h"
#include "mwr/utils/options.h"
#include "mwr/utils/socket.h"
#include "mwr/utils/srec.h"
#include "mwr/utils/terminal.h"
#include "mwr/utils/uimage.h"

#include "mwr/core/version.h"

#endif
