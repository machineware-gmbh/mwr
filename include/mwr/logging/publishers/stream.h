/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_LOGGING_PUBLISHERS_STREAM_H
#define MWR_LOGGING_PUBLISHERS_STREAM_H

#include "mwr/logging/publisher.h"

namespace mwr {
namespace publishers {

class stream : public publisher
{
protected:
    ostream& os;

    virtual void publish(const logmsg& msg) override;

public:
    stream(ostream& os);
    virtual ~stream();
};

} // namespace publishers
} // namespace mwr

#endif
