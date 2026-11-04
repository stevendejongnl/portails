// SPDX-License-Identifier: AGPL-3.0-or-later
#include "crypto.h"

#include <QMessageAuthenticationCode>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <memory>

namespace portails {

namespace {

constexpr int NONCE_SIZE = 12;
constexpr int TAG_SIZE = 16;
const QByteArray MAGIC = QByteArrayLiteral("PTV1");
const QByteArray KEY_LABEL = QByteArrayLiteral("portails/v1/profile-key");
const QByteArray KEYRING_LABEL = QByteArrayLiteral("portails/v2/keyring-key");

using CipherContext = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

CipherContext newContext()
{
    return CipherContext(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
}

const unsigned char *bytes(const QByteArray &data)
{
    return reinterpret_cast<const unsigned char *>(data.constData());
}

unsigned char *writableBytes(QByteArray &data)
{
    return reinterpret_cast<unsigned char *>(data.data());
}

QByteArray associatedData(const QByteArray &context)
{
    return MAGIC + context;
}

bool startCipher(EVP_CIPHER_CTX *context, bool encrypting, const QByteArray &key, const QByteArray &nonce,
                 const QByteArray &associated)
{
    const auto initialize = encrypting ? EVP_EncryptInit_ex : EVP_DecryptInit_ex;
    const auto update = encrypting ? EVP_EncryptUpdate : EVP_DecryptUpdate;
    int length = 0;
    return initialize(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1
        && EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, NONCE_SIZE, nullptr) == 1
        && initialize(context, nullptr, nullptr, bytes(key), bytes(nonce)) == 1
        && update(context, nullptr, &length, bytes(associated), associated.size()) == 1;
}

bool newNonce(QByteArray &nonce)
{
    nonce.resize(NONCE_SIZE);
    return RAND_bytes(writableBytes(nonce), NONCE_SIZE) == 1;
}

bool encryptInto(EVP_CIPHER_CTX *context, const QByteArray &plain, QByteArray &cipherText, QByteArray &tag)
{
    int length = 0;
    int finalLength = 0;
    cipherText.resize(plain.size());
    tag.resize(TAG_SIZE);
    return EVP_EncryptUpdate(context, writableBytes(cipherText), &length, bytes(plain), plain.size()) == 1
        && EVP_EncryptFinal_ex(context, writableBytes(cipherText) + length, &finalLength) == 1
        && EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, writableBytes(tag)) == 1;
}

bool decryptInto(EVP_CIPHER_CTX *context, const QByteArray &cipherText, QByteArray tag, QByteArray &plain)
{
    int length = 0;
    int finalLength = 0;
    plain.resize(cipherText.size());
    return EVP_DecryptUpdate(context, writableBytes(plain), &length, bytes(cipherText), cipherText.size()) == 1
        && EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, writableBytes(tag)) == 1
        && EVP_DecryptFinal_ex(context, writableBytes(plain) + length, &finalLength) == 1;
}

}  // namespace

QByteArray deriveProfileKey(const QByteArray &machineId)
{
    if (machineId.trimmed().isEmpty()) {
        return {};
    }
    return QMessageAuthenticationCode::hash(KEY_LABEL, machineId.trimmed(), QCryptographicHash::Sha256);
}

QByteArray deriveProfileKey(const QByteArray &machineId, const QByteArray &keyringSecret)
{
    const QByteArray machineKey = deriveProfileKey(machineId);
    if (machineKey.isEmpty() || keyringSecret.isEmpty()) {
        return {};
    }
    return QMessageAuthenticationCode::hash(KEYRING_LABEL + machineKey, keyringSecret, QCryptographicHash::Sha256);
}

QByteArray encryptBlob(const QByteArray &key, const QByteArray &plain, const QByteArray &context)
{
    QByteArray nonce;
    QByteArray cipherText;
    QByteArray tag;
    const CipherContext cipher = newContext();
    const bool succeeded = key.size() == KEY_SIZE && newNonce(nonce)
        && startCipher(cipher.get(), true, key, nonce, associatedData(context))
        && encryptInto(cipher.get(), plain, cipherText, tag);
    return succeeded ? MAGIC + nonce + cipherText + tag : QByteArray();
}

std::optional<QByteArray> decryptBlob(const QByteArray &key, const QByteArray &blob, const QByteArray &context)
{
    const int minimumSize = MAGIC.size() + NONCE_SIZE + TAG_SIZE;
    if (key.size() != KEY_SIZE || blob.size() < minimumSize || !blob.startsWith(MAGIC)) {
        return std::nullopt;
    }
    const QByteArray nonce = blob.mid(MAGIC.size(), NONCE_SIZE);
    const QByteArray cipherText = blob.mid(MAGIC.size() + NONCE_SIZE, blob.size() - minimumSize);
    const QByteArray tag = blob.right(TAG_SIZE);
    QByteArray plain;
    const CipherContext cipher = newContext();
    const bool succeeded = startCipher(cipher.get(), false, key, nonce, associatedData(context))
        && decryptInto(cipher.get(), cipherText, tag, plain);
    if (!succeeded) {
        return std::nullopt;
    }
    return plain;
}

}  // namespace portails
