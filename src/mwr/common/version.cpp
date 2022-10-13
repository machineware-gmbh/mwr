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

#include "mwr/common/version.h"

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
