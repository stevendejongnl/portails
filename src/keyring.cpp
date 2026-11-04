// SPDX-License-Identifier: AGPL-3.0-or-later
// GLib headers first: they use the word "signals", which the Qt headers turn into a macro.
#include <libsecret/secret.h>

#include "keyring.h"

#include <openssl/rand.h>

namespace portails {

namespace {

constexpr int SECRET_SIZE = 32;

const SecretSchema *profileKeySchema()
{
    static const SecretSchema schema = {
        "nl.madebysteven.portails.profile-key",
        SECRET_SCHEMA_NONE,
        {{"purpose", SECRET_SCHEMA_ATTRIBUTE_STRING}, {"NULL", SECRET_SCHEMA_ATTRIBUTE_STRING}},
    };
    return &schema;
}

QString takeErrorMessage(GError *error)
{
    const QString message = QString::fromUtf8(error->message);
    g_error_free(error);
    return message;
}

KeyringSecret decodeStoredSecret(const QByteArray &stored)
{
    const QByteArray secret = QByteArray::fromBase64(stored);
    if (secret.size() != SECRET_SIZE) {
        return {{}, QStringLiteral("the keyring entry of portails is damaged")};
    }
    return {secret, {}};
}

KeyringSecret storeNewSecret()
{
    QByteArray secret(SECRET_SIZE, 0);
    if (RAND_bytes(reinterpret_cast<unsigned char *>(secret.data()), SECRET_SIZE) != 1) {
        return {{}, QStringLiteral("no random bytes available")};
    }
    GError *error = nullptr;
    const QByteArray encoded = secret.toBase64();
    const gboolean stored = secret_password_store_sync(profileKeySchema(), SECRET_COLLECTION_DEFAULT,
                                                       "portails profile key", encoded.constData(), nullptr, &error,
                                                       "purpose", "profile-key", nullptr);
    if (!stored) {
        return {{}, error ? takeErrorMessage(error) : QStringLiteral("the keyring did not store the secret")};
    }
    return {secret, {}};
}

}  // namespace

KeyringSecret loadOrCreateKeyringSecret()
{
    GError *error = nullptr;
    gchar *stored = secret_password_lookup_sync(profileKeySchema(), nullptr, &error, "purpose", "profile-key", nullptr);
    if (error) {
        return {{}, takeErrorMessage(error)};
    }
    if (!stored) {
        return storeNewSecret();
    }
    const KeyringSecret result = decodeStoredSecret(QByteArray(stored));
    secret_password_free(stored);
    return result;
}

}  // namespace portails
