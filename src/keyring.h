// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QByteArray>
#include <QString>

namespace portails {

struct KeyringSecret {
    QByteArray secret;
    QString error;
};

// Reads the secret of this app from the desktop keyring (Secret Service) and creates it on first use.
// The secret is 32 random bytes. It never leaves the keyring except in memory.
// An empty secret and a message mean the keyring could not be used.
KeyringSecret loadOrCreateKeyringSecret();

}  // namespace portails
