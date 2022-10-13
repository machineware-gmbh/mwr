/******************************************************************************
 *                                                                            *
 * Copyright 2022 MachineWare GmbH                                            *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is unpublished proprietary work and may not be used or disclosed to   *
 * third parties, copied or duplicated in any form, in whole or in part,      *
 * without prior written permission of the authors.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/version.h"

#ifndef MWR_VERSION_MAJOR
#error MWR_VERSION_MAJOR undefined
#endif

#ifndef MWR_VERSION_MINOR
#error MWR_VERSION_MINOR undefined
#endif

#ifndef MWR_VERSION_PATCH
#error MWR_VERSION_PATCH undefined
#endif

#ifndef MWR_GIT_REV
#error MWR_GIT_REV undefined
#endif

#ifndef MWR_GIT_REV_SHORT
#error MWR_GIT_REV_SHORT undefined
#endif

#ifndef MWR_VERSION
#error MWR_VERSION undefined
#endif

#ifndef MWR_VERSION_STRING
#error MWR_VERSION_STRING undefined
#endif

namespace mwr {

unsigned int version() {
    return MWR_VERSION;
}

const char* version_string() {
    return MWR_VERSION_STRING;
}

} // namespace mwr
