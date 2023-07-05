/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_LOGGING_PUBLISHERS_FILE_H
#define MWR_LOGGING_PUBLISHERS_FILE_H

#include "mwr/logging/publisher.h"

namespace mwr {
namespace publishers {

class file : public publisher
{
private:
    ofstream m_file;

public:
    file(const string& filename);
    virtual ~file();

protected:
    virtual void publish(const logmsg& msg) override;
};

} // namespace publishers
} // namespace mwr

#endif
