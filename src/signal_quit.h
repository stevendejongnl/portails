// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

class QObject;

namespace portails {

// Makes SIGTERM, SIGINT and SIGHUP end the app through the normal quit path,
// so the encrypted profiles are written before the process exits.
void quitOnTerminationSignals(QObject *parent);

}  // namespace portails
