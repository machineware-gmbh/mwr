/******************************************************************************
 *                                                                            *
 * Copyright (C) 2023 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#ifndef MWR_UTILS_LOCALE_H
#define MWR_UTILS_LOCALE_H

#include <clocale>
#include <string>

namespace mwr {

struct locale_guard {
    int cat;
    std::string saved;

    locale_guard(int category, const char* locale):
        cat(category), saved(std::setlocale(category, NULL)) {
        std::setlocale(category, locale);
    }
    ~locale_guard() { std::setlocale(cat, saved.c_str()); }
};

} // namespace mwr

#endif
