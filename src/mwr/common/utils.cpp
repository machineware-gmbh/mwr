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

#include "mwr/common/utils.h"
#include "mwr/common/compiler.h"
#include "mwr/stl/strings.h"

#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <execinfo.h>
#include <cxxabi.h>

namespace mwr {

string dirname(const string& path) {
#ifdef _WIN32
    const char separator = '\\';
#else
    const char separator = '/';
#endif
    size_t i = path.rfind(separator, path.length());
    return (i == string::npos) ? "." : path.substr(0, i);
}

string filename(const string& path) {
#ifdef _WIN32
    const char separator = '\\';
#else
    const char separator = '/';
#endif
    size_t i = path.rfind(separator, path.length());
    return (i == string::npos) ? path : path.substr(i + 1);
}

string filename_noext(const string& path) {
    const string name = filename(path);
    size_t i = name.rfind('.', path.length());
    return (i == string::npos) ? name : name.substr(0, i);
}

string curr_dir() {
    char path[PATH_MAX];
    if (getcwd(path, sizeof(path)) != path)
        MWR_ERROR("cannot read current directory: %s", strerror(errno));
    return string(path);
}

string temp_dir() {
#ifdef _WIN32
    // ToDo: implement tempdir for windows
#else
    return "/tmp/";
#endif
}

string progname() {
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);

    if (len == -1)
        return "unknown";

    path[len] = '\0';
    return path;
}

string username() {
    char uname[255];
    if (getlogin_r(uname, sizeof(uname)))
        return "unknown";
    return uname;
}

vector<string> backtrace(size_t frames, size_t skip) {
    vector<string> sv;

    void* symbols[frames + skip];
    size_t size = (size_t)::backtrace(symbols, frames + skip);
    if (size <= skip)
        return sv;

    sv.resize(size - skip);

    size_t dmbufsz = 256;
    char* dmbuf = (char*)malloc(dmbufsz);
    char** names = ::backtrace_symbols(symbols, size);
    for (size_t i = skip; i < size; i++) {
        char *func = nullptr, *offset = nullptr, *end = nullptr;
        for (char* ptr = names[i]; *ptr != '\0'; ptr++) {
            if (*ptr == '(')
                func = ptr++;
            else if (*ptr == '+')
                offset = ptr++;
            else if (*ptr == ')') {
                end = ptr++;
                break;
            }
        }

        if (!func || !offset || !end) {
            sv[i - skip] = mkstr("<unknown> [%p]", symbols[i]);
            continue;
        }

        *func++ = '\0';
        *offset++ = '\0';
        *end = '\0';

        sv[i - skip] = string(func) + "+" + string(offset);

        int status = 0;
        char* res = abi::__cxa_demangle(func, dmbuf, &dmbufsz, &status);
        if (status == 0)
            sv[i - skip] = string(dmbuf = res) + "+" + string(offset);
    }

    free(names);
    free(dmbuf);

    return sv;
}

} // namespace mwr
