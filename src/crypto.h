// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QByteArray>

#include <optional>

namespace portails {

constexpr int KEY_SIZE = 32;

// HMAC-SHA256 keyed with the machine id over a fixed application label.
// The raw machine id is never used as a key itself.
QByteArray deriveProfileKey(const QByteArray &machineId);

// The same key, mixed with a secret from the desktop keyring. Neither one alone is enough to open a profile.
QByteArray deriveProfileKey(const QByteArray &machineId, const QByteArray &keyringSecret);

// AES-256-GCM. The result is: magic (4) | nonce (12) | cipher text | tag (16).
// The context is authenticated but not stored. A blob only opens with the same context.
// An empty result means the key size is wrong or the cipher failed.
QByteArray encryptBlob(const QByteArray &key, const QByteArray &plain, const QByteArray &context);

// Returns nothing when the key, the context or the data is wrong, or when the data was changed.
std::optional<QByteArray> decryptBlob(const QByteArray &key, const QByteArray &blob, const QByteArray &context);

}  // namespace portails
