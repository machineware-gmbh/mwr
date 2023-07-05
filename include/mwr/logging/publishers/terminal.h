/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_LOGGING_PUBLISHERS_TERM_H
#define MWR_LOGGING_PUBLISHERS_TERM_H

#include "mwr/logging/publisher.h"

namespace mwr {
namespace publishers {

class terminal : public publisher
{
private:
    bool m_colors;
    ostream& m_os;

public:
    bool has_colors() const { return m_colors; }
    void set_colors(bool set = true) { m_colors = set; }

    terminal(bool use_cerr = true);
    terminal(bool use_cerr, bool use_colors);
    virtual ~terminal();

    static const char* colors[NUM_LOG_LEVELS];

protected:
    virtual void publish(const logmsg& msg) override;
};

} // namespace publishers
} // namespace mwr

#endif
