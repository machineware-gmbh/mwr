/******************************************************************************
 *                                                                            *
 * Copyright (C) 2022 MachineWare GmbH                                        *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 *                                                                            *
 ******************************************************************************/

#include "mwr/core/report.h"
#include "mwr/core/utils.h"

#include <stdio.h>
#include <signal.h>

#ifdef MWR_LINUX
#include <unistd.h>
#include <execinfo.h>
#include <cxxabi.h>
#endif

#ifdef MWR_WINDOWS
#include <Windows.h>
#include <DbgHelp.h>
#include <io.h>
#endif

#ifdef MWR_MACOS
#include <unistd.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <mach-o/dyld.h>
#endif

namespace mwr {

ostream& operator<<(ostream& os, const stackframe& frame) {
    if (!frame.symbol.empty()) {
        os << mkstr("[0x%012llx] %s +0x%llx", frame.address,
                    frame.symbol.c_str(), frame.offset);
    } else {
        os << mkstr("[0x%012llx] <unknown>", frame.address);
    }

    return os;
}

vector<stackframe> backtrace(size_t frames, size_t skip) {
    vector<stackframe> sv;

#if defined(MWR_LINUX) || defined(MWR_MACOS)

    vector<void*> symbols(frames + skip);
    size_t size = (size_t)::backtrace(symbols.data(), symbols.size());
    if (size <= skip)
        return sv;

    size_t dmbufsz = 256;
    char* dmbuf = (char*)malloc(dmbufsz);
    char** names = ::backtrace_symbols(symbols.data(), symbols.size());
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

        stackframe frame;
        frame.address = (u64)symbols[i];

        if (func && offset && end) {
            *func++ = '\0';
            *offset++ = '\0';
            *end = '\0';

            frame.offset = strtol(offset, NULL, 16);

            int status = 0;
            char* res = abi::__cxa_demangle(func, dmbuf, &dmbufsz, &status);
            if (status == 0)
                frame.symbol = to_string(dmbuf = res);
            else
                frame.symbol = to_string(func);
        }

        sv.push_back(frame);
    }

    free(names);
    free(dmbuf);

#elif defined(MWR_WINDOWS)

    void* symbols[256];
    if (frames > MWR_ARRAY_SIZE(symbols))
        frames = MWR_ARRAY_SIZE(symbols);

    size_t size = CaptureStackBackTrace((DWORD)skip, (DWORD)frames, symbols,
                                        NULL);

    HANDLE pid = GetCurrentProcess();
    SymInitialize(pid, NULL, TRUE);

    for (size_t i = 0; i < size; i++) {
        stackframe frame;

        frame.address = (u64)symbols[i];
        frame.offset = 0;

        char buffer[sizeof(SYMBOL_INFO) + MAX_PATH];
        SYMBOL_INFO* symbol = (SYMBOL_INFO*)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_PATH;

        if (SymFromAddr(pid, frame.address, &frame.offset, symbol))
            frame.symbol = to_string(symbol->Name);

        sv.push_back(frame);
    }

#endif

    return sv;
}

size_t max_backtrace_length = 16;

vector<stackframe> backtrace(size_t skip) {
    return backtrace(max_backtrace_length + skip, skip);
}

void print_backtrace(const vector<stackframe>& bt, ostream& os) {
    if (bt.empty())
        os << "<backtrace unavailable>" << std::endl;

    for (auto it = bt.rbegin(); it != bt.rend(); it++)
        os << *it << std::endl;
}

void print_backtrace(ostream& os) {
    vector<stackframe> bt = backtrace(3);
    print_backtrace(bt, os);
}

#if defined(MWR_MSVC)
static LPTOP_LEVEL_EXCEPTION_FILTER prev_handler;
static LONG WINAPI handle_exception(EXCEPTION_POINTERS* info) {
    if (info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        print_backtrace(std::cerr);
        fprintf(stderr,
                "EXCEPTION_ACCESS_VIOLATION while accessing memory at %p\n",
                (void*)info->ExceptionRecord->ExceptionInformation[1]);
        fflush(stderr);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    if (prev_handler)
        return prev_handler(info);
    else
        return EXCEPTION_CONTINUE_SEARCH;
}

void report_segfaults() {
    prev_handler = SetUnhandledExceptionFilter(handle_exception);
}

#else
static struct sigaction oldact;
static void handle_segfault(int sig, siginfo_t* info, void* context) {
    print_backtrace(std::cerr);
    fprintf(stderr,
            "Caught signal %d (%s) while accessing memory at location %p\n",
            sig, strsignal(sig), info->si_addr);
    fflush(stderr);

    if ((oldact.sa_flags & SA_SIGINFO) && (oldact.sa_sigaction != NULL)) {
        (*oldact.sa_sigaction)(sig, info, context);
        return;
    }

    if (oldact.sa_handler != NULL) {
        (*oldact.sa_handler)(sig);
        return;
    }

    // If there is no other handler to call, reset the handler back to the
    // default signal handler and then re-raise the signal again.
    signal(sig, SIG_DFL);
    raise(sig);
}

void report_segfaults() {
    struct sigaction newact;
    memset(&newact, 0, sizeof(newact));
    memset(&oldact, 0, sizeof(oldact));

    sigemptyset(&newact.sa_mask);
    newact.sa_sigaction = &handle_segfault;
    newact.sa_flags = SA_SIGINFO;

    if (::sigaction(SIGSEGV, &newact, &oldact) < 0)
        MWR_ERROR("failed to install SIGSEGV signal handler");
}
#endif

report::report(const string& msg, const char* file, size_t line):
    std::exception(),
    m_message(msg),
    m_file(file),
    m_line(line),
    m_backtrace(mwr::backtrace(2)) {
    // nothing to do
}

report::~report() throw() {
    // nothing to do
}

const char* report::what() const throw() {
    return m_message.c_str();
}

ostream& operator<<(ostream& os, const report& rep) {
    os << rep.what();
    return os;
}

} // namespace mwr
