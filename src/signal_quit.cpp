// SPDX-License-Identifier: AGPL-3.0-or-later
#include "signal_quit.h"

#include <QCoreApplication>
#include <QSocketNotifier>

#include <atomic>
#include <csignal>
#include <sys/socket.h>
#include <unistd.h>

namespace portails {

namespace {

int signalPipe[2] = {-1, -1};
std::atomic<bool> quitRequested{false};

void handleSignal(int)
{
    const char marker = 1;
    const ssize_t written = ::write(signalPipe[1], &marker, 1);
    (void)written;
}

void consumeMarker()
{
    char marker = 0;
    const ssize_t received = ::read(signalPipe[0], &marker, 1);
    (void)received;
    // Every later signal is ignored. A second quit() while the app shuts down crashes Qt.
    if (quitRequested.exchange(true)) {
        return;
    }
    QCoreApplication::quit();
}

void installHandler(int signalNumber)
{
    struct sigaction action {};
    action.sa_handler = handleSignal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(signalNumber, &action, nullptr);
}

}  // namespace

void quitOnTerminationSignals(QObject *parent)
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, signalPipe) != 0) {
        return;
    }
    auto *notifier = new QSocketNotifier(signalPipe[0], QSocketNotifier::Read, parent);
    QObject::connect(notifier, &QSocketNotifier::activated, consumeMarker);
    for (const int signalNumber : {SIGTERM, SIGINT, SIGHUP}) {
        installHandler(signalNumber);
    }
}

}  // namespace portails
