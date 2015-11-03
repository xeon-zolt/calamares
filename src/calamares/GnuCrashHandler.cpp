/* === This file is part of Calamares - <http://github.com/calamares> ===
 *
 *   Copyright 2015, Teo Mrnjavac <teo@kde.org>
 *
 *   Calamares is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Calamares is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Calamares. If not, see <http://www.gnu.org/licenses/>.
 */

#include "GnuCrashHandler.h"

#include <QList>
#include <QString>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The code below was obtained from:
// http://stackoverflow.com/questions/77005/how-to-generate-a-stacktrace-when-my-gcc-c-app-crashes/1925461#1925461
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>


GnuCrashHandler* GnuCrashHandler::s_instance = nullptr;


/* This structure mirrors the one found in /usr/include/asm/ucontext.h */
typedef struct _sig_ucontext {
    unsigned long     uc_flags;
    struct ucontext   *uc_link;
    stack_t           uc_stack;
    struct sigcontext uc_mcontext;
    sigset_t          uc_sigmask;
} sig_ucontext_t;


void
crit_err_hdlr(int sig_num, siginfo_t * info, void * ucontext) {
    void *             array[50];
    void *             caller_address;
    char **            messages;
    int                size, i;
    sig_ucontext_t *   uc;

    uc = (sig_ucontext_t *)ucontext;

    /* Get the address at the time the signal was raised */
    #if defined(__i386__) // gcc specific
    caller_address = (void *) uc->uc_mcontext.eip; // EIP: x86 specific
    #elif defined(__x86_64__) // gcc specific
    caller_address = (void *) uc->uc_mcontext.rip; // RIP: x86_64 specific
    #else
    #error Unsupported architecture. // TODO: Add support for other arch.
    #endif

    fprintf(stderr, "\n");
    FILE * backtraceFile;

    // In this example we write the stacktrace to a file. However, we can also just fprintf to stderr (or do both).
    QString backtraceFilePath = GnuCrashHandler::instance()->backtracePath();
    backtraceFile = fopen(backtraceFilePath.toUtf8().data(),"w");

    if (sig_num == SIGSEGV)
        fprintf(backtraceFile, "signal %d (%s), address is %p from %p\n",sig_num, strsignal(sig_num), info->si_addr,(void *)caller_address);
    else
        fprintf(backtraceFile, "signal %d (%s)\n",sig_num, strsignal(sig_num));

    size = backtrace(array, 50);
    /* overwrite sigaction with caller's address */
    array[1] = caller_address;
    messages = backtrace_symbols(array, size);
    /* skip first stack frame (points here) */
    for (i = 1; i < size && messages != NULL; ++i) {
        fprintf(backtraceFile, "[bt]: (%d) %s\n", i, messages[i]);
    }

    fclose(backtraceFile);
    free(messages);

    exit(EXIT_FAILURE);
}


// GnuCrashHandler code begins here.

GnuCrashHandler*
GnuCrashHandler::instance()
{
    return s_instance;
}


void
GnuCrashHandler::install( const QString& backtracePath,
                          const QList<int>& signalsToCatch )
{
    if ( s_instance )
        fprintf( stderr, "ERROR: cannot install more than one crash handler." );
    s_instance = new GnuCrashHandler( backtracePath, signalsToCatch );
}


void
GnuCrashHandler::installSignal(int __sig) {
    struct sigaction sigact;
    sigact.sa_sigaction = crit_err_hdlr;
    sigact.sa_flags = SA_RESTART | SA_SIGINFO;
    if (sigaction(__sig, &sigact, (struct sigaction *)NULL) != 0) {
        fprintf(stderr, "error setting signal handler for %d (%s)\n",__sig, strsignal(__sig));
        exit(EXIT_FAILURE);
    }
}


GnuCrashHandler::GnuCrashHandler( const QString& backtracePath,
                                  const QList<int>& signalsToCatch )
    : m_backtracePath( backtracePath )
{
    for ( int sig : signalsToCatch )
        installSignal( sig );
}
